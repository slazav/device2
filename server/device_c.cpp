#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h> // usleep

#include <curl/curl.h>
#include "tun.h"

#include "read_words/read_words.h"
#include "read_words/read_conf.h"
#include "getopt/getopt.h"
#include "getopt/help_printer.h"
#include "err/err.h"


/*************************************************/
// print help message
void usage(const GetOptSet & options, bool pod=false){
  HelpPrinter pr(pod, options, "device_c");
  pr.name("device client program");
  pr.usage("[<options>] ask <dev> <msg> -- send message to the device, print answer");
  pr.usage("[<options>] use_dev <dev>   -- SPP interface to a device");
  pr.usage("[<options>] use_srv         -- SPP interface to the server");
  pr.usage("[<options>] (list|devices)  -- print list of available devices");
  pr.usage("[<options>] info <dev>      -- print information about device");
  pr.usage("[<options>] reload          -- reload device configuration");
  pr.usage("[<options>] close <dev>     -- close device (it will be reopened if needed)");
  pr.usage("[<options>] monitor <dev>   -- monitor all communication of the device");
  pr.usage("[<options>] ping            -- check if the server is working");
  pr.usage("[<options>] get_time        -- get server system time");
  pr.usage("[<options>] get_srv         -- get server address");

  pr.head(1, "Options:");
  pr.opts({"DEVCLI"});
  pr.par("Server program: device_d(1).");
  pr.par("Homepage, documentation: https://github.com/slazav/device2");
  pr.par("V.Zavjalov, 2020, slazav at altlinux dot org");
  throw Err();
}

// write callback for libcurl
size_t write_cb(void *buffer, size_t size, size_t nmemb, void *data){
  *(std::string*)data += std::string((const char*)buffer, size*nmemb);
  return size*nmemb;
}

class Downloader {
  CURLM *cm;
  std::string server;

public:

  Downloader(const std::string & srv): server(srv){
    curl_global_init(CURL_GLOBAL_ALL);
    cm = curl_easy_init();
  }

  ~Downloader(){
    curl_easy_cleanup(cm);
  }

  // ask the server
  std::string get(const std::string & act,
                  const std::string & dev = "",
                  const std::string & cmd = ""){
    // escape url components
    char *dev_ = curl_easy_escape(cm, dev.data() , dev.size());
    char *act_ = curl_easy_escape(cm, act.data() , act.size());
    char *cmd_ = curl_easy_escape(cm, cmd.data() , cmd.size());

    // build url, free unneeded strings
    std::string url = server + "/" + act_;
    if  (dev != "") url += std::string("/") + dev_;
    if  (cmd != "") url += std::string("/") + cmd_;
    curl_free(dev_);
    curl_free(act_);
    curl_free(cmd_);

    // set curl options
    std::string data; // data storage
    curl_easy_setopt(cm, CURLOPT_URL, url.c_str());
    curl_easy_setopt(cm, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(cm, CURLOPT_WRITEDATA, (void*) &data);
    auto ret = curl_easy_perform(cm);
    if (ret != CURLE_OK) throw Err() << curl_easy_strerror(ret);

    // get response code
    long http_code = 0;
    curl_easy_getinfo (cm, CURLINFO_RESPONSE_CODE, &http_code);
    if (http_code != 200) throw Err() << data;
    return data;
  }

  // SPP interface to a single device.
  void use_dev(const std::string & dev,
               std::istream & in, std::ostream & out,
               const bool lock, const std::string & name){

    out << "#SPP001\n"; // command-line protocol, version 001.
    out << "Server: " << server << "\n";
    out << "Device: " << dev << "\n";

    // Outer try -- exit on errors with #Error message
    // For SPP2 it should be #Fatal
    try {
      // open device, set lock and connection name,
      // throw error if needed
      get("use", dev);
      if (name !="") get("set_conn_name", name);
      if (lock) get("lock", dev);
      out << "#OK\n";
      out.flush();

      while (1){
        // inner try -- continue to a new command with #Error message
        try {
          if (!in) break;
          auto pars = read_words(in);
          if (pars.size()==0) break;
          out << get("ask", dev, join_words(pars)) << '\n';
          out << "#OK\n";
          out.flush();
        }
        catch(Err e){
          if (e.str()!="") out << "#Error: " << e.str() << "\n";
          out.flush();
        }
      }
    }
    catch(Err e){
      if (e.str()!="") out << "#Error: " << e.str() << "\n";
      return;
    }
    get("release_all");
    return;
  }

  // SPP interface to the server.
  void use_srv(std::istream & in, std::ostream & out){
    out << "#SPP001\n"; // command-line protocol, version 001.
    out << "Server: " << server << "\n";
    out.flush();

    // Outer try -- exit on errors with #Error message
    // For SPP2 it should be #Fatal
    try {
      // ping the server, throw error if needed
      get("ping");
      out << "#OK\n";
      out.flush();

      while (1){
        // inner try -- continue to a new command with #Error message
        try {
          if (!in) break;
          auto pars = read_words(in);
          if (pars.size()==0) break;

          if (pars.size()>3)
            throw Err() << "too many arguments";
          pars.resize(3);

          out << get(pars[0], pars[1], pars[2]) << '\n';
          out << "#OK\n";
          out.flush();
        }
        catch(Err e){
          if (e.str()!="") out << "#Error: " << e.str() << "\n";
          out.flush();
        }
      }
    }
    catch(Err e){
      if (e.str()!="") out << "#Error: " << e.str() << "\n";
      return;
    }
    get("release_all");
    return;
  }

  void monitor(const std::string & dev, std::ostream & out){
    get("log_start", dev);
    while(1){
      out << get("log_get", dev);
      usleep(100000);
    }
  }

};

/*************************************************/
void
check_par_count(const std::vector<std::string> & pars,
                const int num) {
  if (num < 1)
    throw Err() << "bad usage of check_par_count()";

  if (pars.size()>num)
    throw Err() << "unexpected parameter for \""
                << pars[0] << "\" action: " << pars[num];

  if (pars.size()<num)
    throw Err() << "not enough parameters for \""
                << pars[0] << "\" action";
}

/*************************************************/
// main function.

int
main(int argc, char ** argv) {

  try {
    // fill option structure
    GetOptSet options;
    std::string on("DEVCLI");
    options.add("server",  1,'s', on, "Server (default: localhost).");
    options.add("port",    1,'p', on, "Port (default: 8082).");
    options.add("via",     1,'v', on, "Connect to the server through a tunnel. "
                                      "Argument: name of the gateway.");
    options.add("via_cmd", 1,0  , on, "Specify command template for making the tunnel, with $L, $R, $H, "
                                      "and $G for local port, remote port, remote host and gateway. "
                                      "Default: /usr/bin/ssh -f -L \"$L\":\"$H\":\"$R\" \"$G\" sleep 20");
    options.add("lock",    0,'l', on, "Lock the device (only for use_dev action).");
    options.add("name",    0,'n', on, "Set connection name (only for use_dev action). "
                                      "Default: \"device_c(<pid>)\". If empty, reset to server default name");
    options.add("help",    0,'h', on, "Print help message and exit.");
    options.add("pod",     0,0,   on, "Print help message in POD format and exit.");

    // parse options
    Opt opts = parse_options(&argc, &argv, options, {}, 0);
    std::vector<std::string> pars(argv, argv+argc);

    // print help message
    if (opts.exists("help")) usage(options);
    if (opts.exists("pod"))  usage(options,true);

    // read config file
    std::string cfgfile = "/etc/device2/device_c.cfg";
    Opt optsf = read_conf(cfgfile, {"server", "port", "via", "via_cmd"});
    opts.put_missing(optsf);

    // extract parameters
    auto server = opts.get("server", "localhost");
    int port = opts.get("port", 8082);
    auto srv = "http://" + server + ":" + type_to_str(port);
    auto name = opts.get("name",
      std::string("device_c(") + type_to_str(getpid())+")");

    // create a tunnel if needed
    if (opts.exists("via")) srv = create_tunnel(opts);

    // some non-option arguments needed!
    if (pars.size()==0) usage(options);
    auto & action = pars[0];

    Downloader D(srv);

    if (action == "ask"){
      if (pars.size()<3)
        throw Err() << "not enough parameters for \"ask\" action";
      std::vector<std::string> args(pars.begin()+2, pars.end());
      std::cout << D.get(action, pars[1], join_words(args)) << "\n";
      D.get("release", pars[1]);
      return 0;
    }

    if (action == "use_dev"){
      check_par_count(pars, 2);
      D.use_dev(pars[1], std::cin, std::cout, opts.exists("lock"), name);
      return 0;
    }

    if (action == "use_srv"){
      check_par_count(pars, 1);
      D.use_srv(std::cin, std::cout);
      return 0;
    }

    if (action == "list" || action == "devices") {
      check_par_count(pars, 1);
      std::cout << D.get(action);
      return 0;
    }

    if (action == "monitor") {
      check_par_count(pars, 2);
      D.monitor(pars[1], std::cout);
      return 0;
    }

    if (action == "reload") {
      check_par_count(pars, 1);
      std::cout << D.get(action) << "\n";
      return 0;
    }

    if (action == "close"){
      check_par_count(pars, 2);
      D.get(action, pars[1]);
      return 0;
    }

    if (action == "info"){
      check_par_count(pars, 2);
      std::cout << D.get(action, pars[1]);
      return 0;
    }

    if (action == "ping"){
      check_par_count(pars, 1);
      auto ret = D.get(action);
      if ( ret != "")
        throw Err() << "wrong response from the server: " << ret;
      return 0;
    }

    if (action == "get_time"){
      check_par_count(pars, 1);
      std::cout << D.get(action) << "\n";
      return 0;
    }

    if (action == "get_srv"){
      check_par_count(pars, 1);
      std::cout << srv << "\n";
      return 0;
    }

    throw Err() << "unknown action: " << action;

  }
  catch (Err e){
    if (e.str()!="") std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}
