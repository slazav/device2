#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h> // usleep

#include <curl/curl.h>

#include "read_words/read_words.h"
#include "read_words/read_conf.h"
#include "getopt/getopt.h"
#include "getopt/getopt.h"
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
  pr.usage("[<options>] monitor <dev>   -- monitor all communication of the device");
  pr.usage("[<options>] ping            -- check if the server is working");
  pr.usage("[<options>] get_time        -- get server system time");

  pr.head(1, "Options:");
  pr.opts({"DEVCLI"});
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
  void use_dev(const std::string & dev, std::istream & in, std::ostream & out, const bool lock){
    out << "#SPP001\n"; // command-line protocol, version 001.
    out << "Server: " << server << "\n";
    out << "Device: " << dev << "\n";
    if (lock) get("lock", dev);

    out.flush();

    // Outer try -- exit on errors with #Error message
    // For SPP2 it should be #Fatal
    try {
      // open device, throw error if needed
      get("use", dev);
      out << "#OK\n";
      out.flush();

      while (1){
        // inner try -- continue to a new command with #Error message
        try {
          if (!in) break;
          std::string arg;
          getline(in, arg);
          if (arg.size()==0) continue;
          out << get("ask", dev, arg) << '\n';
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
    get("release", dev);
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
    options.add("server",  1,'s', on, "Server (default: http://localhost).");
    options.add("port",    1,'p', on, "Port (default: 8082).");
    options.add("lock",    0,'l', on, "Lock the device (only for use_dev action).");
    options.add("help",    0,'h', on, "Print help message and exit.");
    options.add("pod",     0,0,   on, "Print help message in POD format and exit.");

    // parse options
    std::vector<std::string> pars;
    Opt opts = parse_options_all(&argc, &argv, options, {}, pars);

    // print help message
    if (opts.exists("help")) usage(options);
    if (opts.exists("pod"))  usage(options,true);

    // read config file
    std::string cfgfile = "/etc/device2/device_c.cfg";
    Opt optsf = read_conf(cfgfile, {"server", "port"});
    opts.put_missing(optsf);

    // extract parameters
    std::string server = opts.get("server", "http://localhost");
    int port = opts.get("port", 8082);
    auto srv = server + ":" + type_to_str(port);

    if (pars.size()==0) usage(options);
    auto & action = pars[0];

    Downloader D(srv);

    if (action == "ask"){
      check_par_count(pars, 3);
      std::cout << D.get(action, pars[1], pars[2]) << "\n";
      return 0;
    }

    if (action == "use_dev"){
      check_par_count(pars, 2);
      D.use_dev(pars[1], std::cin, std::cout, opts.exists("lock"));
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

    throw Err() << "unknown action: " << action;

  }
  catch (Err e){
    if (e.str()!="") std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}
