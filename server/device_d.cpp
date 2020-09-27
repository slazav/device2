#include <iostream>
#include <fstream>
#include <string>

#include <csignal>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h> // wait


#include "getopt/getopt.h"
#include "err/err.h"
#include "log/log.h"
#include "dev_manager.h"
#include "http_server.h"


/*************************************************/
// print help message
void usage(const GetOptSet & options, bool pod=false){
  HelpPrinter pr(pod, options, "device_d");
  pr.name("device server");
  pr.usage("<options>");

  pr.head(1, "Options:");
  pr.opts({"DEVSERV"});
  throw Err();
}

/*************************************************/
// signal handler
static void StopFunc(int signum){ throw 0; }

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
    options.add("config",  1,'C', "DEVSERV", "Device configuration file (default: /etc/device/devices.cfg).");
    options.add("port",    1,'p', "DEVSERV", "TCP port for connections (default: 8082).");
    options.add("dofork",  0,'f', "DEVSERV", "Do fork and run as a daemon.");
    options.add("stop",    0,'S', "DEVSERV", "Stop running daemon (found by pid-file).");
    options.add("verbose", 1,'v', "DEVSERV", "Verbosity level: "
      "0 - write nothing; "
      "1 - write some information on server start/stop; "
      "2 - write about opening/closing connections and devices; "
      "3 - write all messages sent to devices and recieved from them. "
      " (default: 1).");
    options.add("logfile", 1,'l', "DEVSERV", "Log file, '-' for stdout. "
      "(default: /var/log/device_d.log in daemon mode, '-' in console mode.");
    options.add("pidfile", 1,'P', "DEVSERV", "Pid file (default: /var/run/device_d.pid)");
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

    // extract parameters
    int port    = opts.get("port", 8082);
    bool dofork = opts.exists("dofork");
    bool stop   = opts.exists("stop");
    int  verb   = opts.get("verbose", 1);
    logfile = opts.get("logfile");
    pidfile = opts.get("pidfile", "/var/run/device_d.pid");
    devfile = opts.get("config", "/etc/device2/devices.cfg");

    // default log file
    if (logfile==""){
      if (dofork) logfile="/var/log/device_d.log";
      else logfile="-";
    }
    Log::set_log_file(logfile);
    Log::set_log_level(verb);

    // stop running daemon
    if (stop) {
      std::ifstream pf(pidfile);
      if (pf.fail())
        throw Err() << "can't open pid-file: " << pidfile;
      int pid;
      pf >> pid;

      if (kill(pid, SIGTERM) == 0){
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
        throw Err() << "device_d already runing (pid-file exists): " << pid;
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
    DevManager dm;

    // read configuration file
    dm.read_conf(devfile);

    HTTP_Server srv(port, &dm);
    Log(1) << "Starting HTTP server at port " << port;

    // set up signals
    {
      struct sigaction sa;
      sa.sa_handler = StopFunc;
      sigemptyset(&sa.sa_mask);
      sa.sa_flags = SA_RESTART; // Restart functions if interrupted by handler
      if (sigaction(SIGTERM, &sa, NULL) == -1 ||
          sigaction(SIGQUIT, &sa, NULL) == -1 ||
          sigaction(SIGINT,  &sa, NULL) == -1 ||
          sigaction(SIGHUP,  &sa, NULL) == -1)
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
