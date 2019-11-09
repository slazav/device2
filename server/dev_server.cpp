#include <iostream>
#include <fstream>
#include <string>

#include <csignal>
#include <sys/types.h>
#include <sys/stat.h>

#include "getopt/getopt.h"
#include "err/err.h"
#include "dev_manager.h"
#include "http_server.h"


/*************************************************/
// print help message
void usage(const GetOptSet & options, bool pod=false){
  HelpPrinter pr(pod, options, "dev_server");
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

  // log streams
  std::ostream *log = &std::cout;  // log stream
  std::ofstream flog; // log stream for file logs

  std::string logfile; // fog file name
  std::string pidfile; // pidfile
  std::string devfile; // device configuration file

  int ret; // return code

  try {

    // fill option structure
    GetOptSet options;
    options.add("config",  1,'C', "DEVSERV", "Device configuration file (default: /etc/dev_server.txt).");
    options.add("port",    1,'p', "DEVSERV", "TCP port for connections (default: 8082).");
    options.add("daemon",  0,'d', "DEVSERV", "Do fork and run as a daemon.");
    options.add("stop",    0,'S', "DEVSERV", "Stop running daemon (found by pid-file).");
    options.add("verbose", 1,'v', "DEVSERV", "Verbosity level: 0 - write nothing; "
      "1 - write some information on start; 2 - write information about connections; "
      "3 - write input data; 4 - write output data (default: 0).");
    options.add("logfile", 1,'L', "DEVSERV", "Log file, '-' for stdout. "
      "(default: /var/log/dev_server.log in daemon mode, '-' in console mode.");
    options.add("pidfile", 1,'P', "DEVSERV", "Pid file (default: /var/run/dev_server.pid)");
    options.add("help",    0,'h', "DEVSERV", "Print help message.");
    options.add("pod",     0,0,   "DEVSERV", "Print help message in POD format.");

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
    bool dofork = opts.exists("daemon");
    bool stop   = opts.exists("stop");
    int  verb   = opts.get("verbose", 0);
    logfile = opts.get("logfile");
    pidfile = opts.get("pidfile", "/var/run/dev_server.pid");
    devfile = opts.get("config", "/etc/dev_server.txt");

    // default log file
    if (logfile==""){
      if (dofork) logfile="/var/log/dev_server.log";
      else logfile="-";
    }

    // open log file
    if (logfile!="-"){
      flog.open(logfile, std::ios::app);
      if (flog.fail())
        throw Err() << "can't open log file: " << logfile;
      log = &flog;
    }

    // stop running daemon
    if (stop) {
      std::ifstream pf(pidfile);
      if (pf.fail())
        throw Err() << "can't open pid-file: " << pidfile;
      int pid;
      pf >> pid;
      if (kill(pid, SIGTERM) != 0)
        throw Err() << "can't stop dev_server process " << pid << ": "
                    << strerror(errno);
      return 0;
    }

    // check pid file
    {
      std::ifstream pf(pidfile);
      if (!pf.fail()){
        int pid;
        pf >> pid;
        throw Err() << "dev_server already runing (pid-file exists): " << pid;
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
        // write pidfile and exit
        std::ofstream pf(pidfile);
        if (pf.fail())
          throw Err() << "can't open pid-file: " << pidfile;
        pf << pid;
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
    else {
      pid_t pid = getpid();
      std::ofstream pf(pidfile);
      if (pf.fail())
        throw Err() << "can't open pid-file: " << pidfile;
      pf << pid;
    }

    // create device manager
    DevManager dm;

    // read configuration file
    dm.read_conf(devfile);

    HTTP_Server srv(port, dm);
    if (verb>0) *log << "Starting the server at port " << port << "\n";

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

    if (verb>0) *log << "Stopping the server\n";
    ret=0;
  }
  catch (Err e){
    if (e.str()!="") *log << "Error: " << e.str() << "\n";
    ret = e.code();
    if (ret==-1) ret=1; // default code?!
  }

  remove(pidfile.c_str()); // try to remove pid-file
  return ret;
}
