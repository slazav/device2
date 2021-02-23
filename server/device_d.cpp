#include <iostream>
#include <fstream>
#include <string>

#include <csignal>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h> // wait
#include <unistd.h>
#include <pwd.h>

#include "getopt/getopt.h"
#include "read_words/read_conf.h"
#include "err/err.h"
#include "log/log.h"
#include "dev_manager.h"
#include "http_server.h"

#define DEF_CFGFILE "/etc/device2/device_d.cfg"
#define DEF_DEVFILE "/etc/device2/devices.cfg"
#define DEF_PIDFILE "/var/run/device_d.pid"
#define DEF_LOGFILE "/var/log/device_d.log"
#define DEF_ADDR    "127.0.0.1"
#define DEF_PORT    8082
#define DEF_VERB    1

#define STR(s) STR_(s)
#define STR_(s) #s

/*************************************************/
// print help message
void usage(const GetOptSet & options, bool pod=false){
  HelpPrinter pr(pod, options, "device_d");
  pr.name("device server");
  pr.usage("<options>");

  pr.head(1, "Options:");
  pr.opts({"DEVSERV"});

  pr.par("Client program: device_c(1).");
  pr.par("Homepage, documentation: https://github.com/slazav/device2");
  pr.par("V.Zavjalov, 2020, slazav at altlinux dot org");
  throw Err();
}

/*************************************************/
// signal handlers
static void StopFunc(int signum){ throw 0; }

DevManager * dmp = NULL;
static void ReloadFunc(int signum){ if (dmp) dmp->read_conf(); }

/*************************************************/
// main function.

int
main(int argc, char ** argv) {

  std::string logfile; // log file name
  std::string pidfile; // pidfile
  std::string devfile; // device configuration file

  int ret; // return code
  bool mypid = false; // was the pidfile created by this process?

  try {

    // fill option structure
    GetOptSet options;
    options.add("cfgfile", 1,'C', "DEVSERV", "Server configuration file (default: " DEF_CFGFILE ").");
    options.add("devfile", 1,'D', "DEVSERV", "Device list file (default: " DEF_DEVFILE ").");
    options.add("addr",    1,'a', "DEVSERV", "IP address to listen. Use '*' to listen everywhere (default: " DEF_ADDR ").");
    options.add("port",    1,'p', "DEVSERV", "TCP port for connections (default: " STR(DEF_PORT) ").");
    options.add("dofork",  0,'f', "DEVSERV", "Do fork and run as a daemon.");
    options.add("stop",    0,'S', "DEVSERV", "Stop running daemon (found by pid-file).");
    options.add("reload",  0,'R', "DEVSERV", "Reload configuration of running daemon (found by pid-file).");
    options.add("user",    1,'U', "DEVSERV", "Run as user (default: empty, do not switch user).");
    options.add("verbose", 1,'v', "DEVSERV", "Verbosity level: "
      "0 - write nothing; "
      "1 - write some information on server start/stop; "
      "2 - write about opening/closing connections and devices; "
      "3 - write all messages sent to devices and received from them. "
      " (default: " STR(DEF_VERB) ").");
    options.add("logfile", 1,'l', "DEVSERV", "Log file, '-' for stdout. "
      "(default: " DEF_LOGFILE " in daemon mode, '-' in console mode.");
    options.add("pidfile", 1,'P', "DEVSERV", "Pid file (default: " DEF_PIDFILE ")");
    options.add("test",    0,0,   "DEVSERV", "Test mode with connection number limited to 1.");
    options.add("help",    0,'h', "DEVSERV", "Print help message and exit.");
    options.add("pod",     0,0,   "DEVSERV", "Print help message in POD format and exit.");

    // parse options
    std::vector<std::string> nonopt;
    Opt opts = parse_options_all(&argc, &argv, options, {}, nonopt);
    if (nonopt.size()>0) throw Err()
      << "unexpected argument: " << nonopt[0];

    // print help message
    if (opts.exists("help")) usage(options);
    if (opts.exists("pod"))  usage(options,true);

    // read config file
    std::string cfgfile = opts.get("cfgfile", DEF_CFGFILE);
    Opt optsf = read_conf(cfgfile,
       {"addr", "port","logfile","pidfile","devfile","user","verbose"});
    opts.put_missing(optsf);

    // extract parameters
    std::string addr = opts.get("addr", DEF_ADDR);
    int port    = opts.get("port", DEF_PORT);
    bool dofork = opts.exists("dofork");
    bool stop   = opts.exists("stop");
    bool reload = opts.exists("reload");
    int  verb   = opts.get("verbose", DEF_VERB);
    logfile = opts.get("logfile");
    pidfile = opts.get("pidfile", DEF_PIDFILE);
    devfile = opts.get("devfile", DEF_DEVFILE);
    bool test = opts.get("test", false);
    std::string user = opts.get("user", "");

    // switch user if needed
    if (user!=""){
      struct passwd *p;
      if ((p = getpwnam(user.c_str())) == NULL) throw Err()
         << "unknown user: " << user;
      if (setuid(p->pw_uid)!=0) throw Err()
         << "can't switch to another user: " << user << ": " << strerror(errno);
    }

    // default log file
    if (logfile==""){
      if (dofork) logfile = DEF_LOGFILE;
      else logfile="-";
    }
    Log::set_log_file(logfile);
    Log::set_log_level(verb);

    // stop running daemon
    if (stop || reload) {
      std::ifstream pf(pidfile);
      if (pf.fail())
        throw Err() << "can't open pid-file: " << pidfile;
      int pid;
      pf >> pid;

      if (kill(pid, stop? SIGTERM : SIGHUP) == 0){
        int st=0;
        waitpid(pid, &st, 0);
      }
      else {
        if (errno == ESRCH){ // no such process, we should remove the pid-file
          remove(pidfile.c_str());
        }
        else {
          throw Err() << "can't stop device_d process " << pid << ": "
                      << strerror(errno);
        }
      }
      return 0;
    }

    // check pid file
    {
      std::ifstream pf(pidfile);
      if (!pf.fail()){
        int pid;
        pf >> pid;
        throw Err() << "device_d already running (pid-file exists): " << pid;
      }
    }

    // set up daemon mode
    if (dofork){

      // Fork off the parent process
      pid_t pid, sid;
      pid = fork();
      if (pid < 0)
        throw Err() << "can't do fork";
      if (pid > 0){
        return 0;
      }

      // Change the file mode mask
      umask(0);

      // Create a new SID for the child process
      sid = setsid();
      if (sid < 0)
        throw Err() << "can't get SID";

      // // Change the current working directory
      // if ((chdir("/")) < 0)
      //   throw Err() << "can't do chdir";

      // Close out the standard file descriptors
      close(STDIN_FILENO);
      close(STDOUT_FILENO);
      close(STDERR_FILENO);
    }

    // write pidfile
    {
      pid_t pid = getpid();
      std::ofstream pf(pidfile);
      if (pf.fail())
        throw Err() << "can't open pid-file: " << pidfile;
      pf << pid;
      mypid = true;

      if (dofork)
        Log(1) << "Starting device_d in daemon mode, pid=" << pid;
      else
        Log(1) << "Starting device_d in console mode, pid=" << pid;
    }

    // create device manager
    DevManager dm(devfile);
    dmp = &dm; // pointer for ReloadFunc

    HTTP_Server srv(addr, port, test, &dm);
    Log(1) << "HTTP server is running at "
      << addr << ":" << port;
    if (test) Log(1) << "TESTING MODE";

    // set up signals
    {
      struct sigaction sa;
      sa.sa_handler = StopFunc;
      sigemptyset(&sa.sa_mask);
      sa.sa_flags = SA_RESTART; // Restart functions if interrupted by handler
      if (sigaction(SIGTERM, &sa, NULL) == -1 ||
          sigaction(SIGQUIT, &sa, NULL) == -1 ||
          sigaction(SIGINT,  &sa, NULL) == -1)
        throw Err() << "can't set signal handler";

      sa.sa_handler = ReloadFunc;
      if (sigaction(SIGHUP,  &sa, NULL) == -1)
        throw Err() << "can't set signal handler";
    }

    // main loop (to be interrupted by StopFunc)
    try{ while(1) sleep(10); }
    catch(int ret){}

    Log(1) << "Stopping HTTP server";
    ret=0;
  }
  catch (Err e){
    if (e.str()!="") Log(0) << "Error: " << e.str();
    ret = e.code();
    if (ret==-1) ret=1; // default code?!
  }

  if (mypid && pidfile!= "") remove(pidfile.c_str()); // try to remove pid-file
  return ret;
}
