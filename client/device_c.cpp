#include <iostream>
#include <fstream>
#include <string>

#include <curl/curl.h>

#include "getopt/getopt.h"
#include "err/err.h"


/*************************************************/
// print help message
void usage(const GetOptSet & options, bool pod=false){
  HelpPrinter pr(pod, options, "device_c");
  pr.name("device client program");
  pr.usage("<options>");

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

  std::string get(const std::string & dev, const std::string & act, const std::string & cmd = ""){
    // escape url components
    char *dev_ = curl_easy_escape(cm, dev.data() , dev.size());
    char *act_ = curl_easy_escape(cm, act.data() , act.size());
    char *cmd_ = curl_easy_escape(cm, cmd.data() , cmd.size());

    // build url, free unneded strings
    std::string url = server + "/" + dev_ + "/" + act_;
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

  void interactive(const std::string & dev, std::istream & in, std::ostream & out){
    out << "#SPP001\n"; // command-line protocol, version 001.
    out << "Device: " << dev << "\n";
    out.flush();

    // Outer try -- exit on errors with #Error message
    // For SPP2 it should be #Fatal
    try {
      // open device, throw error if needed
      if (dev!="SERVER") get("SERVER", "open", dev);
      out << "#OK\n";
      out.flush();

      while (1){
        // inner try -- continue to a new command with #Error message
        try {
          if (!in) break;
          std::string arg;
          getline(in, arg);
          if (arg.size()==0) continue;
          out << get(dev, "ask", arg) << '\n';
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


};


/*************************************************/
// main function.

int
main(int argc, char ** argv) {

  try {
    // fill option structure
    GetOptSet options;
    std::string on("DEVCLI");
    options.add("server",  1,'s', on, "Server (default: http://localhost:8082).");
    options.add("list",    0,'l', on, "Print list of available devices.");
    options.add("info",    1,'i', on, "Print device information.");
    options.add("dev",     1,'d', on, "Use device.");
    options.add("action",  1,'a', on, "Action (default: ask)");
    options.add("cmd",     1,'c', on, "Action argument (if not set then interactive mode is used).");
    options.add("help",    0,'h', on, "Print help message.");
    options.add("pod",     0,0,   on, "Print help message in POD format.");

    // parse options
    std::vector<std::string> nonopt;
    Opt opts = parse_options_all(&argc, &argv, options, {}, nonopt);
    if (nonopt.size()>0) throw Err()
      << "unexpected argument: " << nonopt[0];

    // print help message
    if (opts.exists("help")) usage(options);
    if (opts.exists("pod"))  usage(options,true);

    // extract parameters
    std::string server = opts.get("server", "http://localhost:8082");
    std::string action = opts.get("action", "ask");
    std::string cmd    = opts.get("cmd",    "");
    std::string dev    = opts.get("dev",    "");

    Downloader D(server);

    opts.check_conflict({"list", "dev", "info"});

    if (opts.exists("list")){
      std::cout << D.get("SERVER", "list");
      return 0;
    }

    if (opts.exists("info")){
      std::cout << D.get("SERVER", "info", opts.get("info"));
      return 0;
    }

    if (opts.exists("dev")){
      if (opts.exists("cmd")){
        std::cout << D.get(dev, action, cmd) << '\n';
        return 0;
      }
      else{
        D.interactive(dev, std::cin, std::cout);
        return 0;
      }
    }
    usage(options);

  }
  catch (Err e){
    if (e.str()!="") std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}
