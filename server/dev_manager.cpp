#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <unistd.h>

#include "err/err.h"
#include "log/log.h"
#include "read_words/read_words.h"
#include "drivers.h"
#include "dev_manager.h"

#define SRVDEV "SERVER"

/*************************************************/
std::vector<std::string>
DevManager::parse_url(const std::string & url){
  std::vector<std::string> ret(3);
  size_t p1(0), i(0);
  if (url.size()>0 && url[0]=='/') p1++; // skip leading /
  for (i=0; i<2; ++i){
    size_t p2 = url.find('/', p1);
    if (p2 == std::string::npos) break;
    ret[i] = url.substr(p1, p2-p1);
    p1=p2+1;
  }
  ret[i] = url.substr(p1);
  return ret;
}

/*************************************************/
void
DevManager::conn_open(const uint64_t conn){
  Log(2) << "conn:" << conn << " open connection";
}

void
DevManager::conn_close(const uint64_t conn){
  // go through all devices, close ones which are not needed
  for (auto & d:devices) d.second.release(conn);
  Log(2) << "conn:" << conn << " close connection";
}

/*************************************************/
std::string
DevManager::run(const std::string & url, const Opt & opts, const uint64_t conn){
  auto vs = parse_url(url);
  std::string act = vs[0];
  std::string arg = vs[1];
  std::string msg = vs[2];

  // ask/<name>/<cmd> -- send a command to the device, get answer
  if (act == "ask") {
    if (arg=="")
      throw Err() << "device name expected: " << url;
    auto lk = get_sh_lock();
    if (devices.count(arg) == 0)
      throw Err() << "unknown device: " << arg;
    Device & d = devices.find(arg)->second;
    d.use(conn);
    return d.ask(msg);
  }

  // use/<name> -- notify server that device should be open
  if (act == "use") {
    if (arg=="")
      throw Err() << "device name expected: " << url;
    auto lk = get_sh_lock();
    if (devices.count(arg) == 0)
      throw Err() << "unknown device: " << arg;
    devices.find(arg)->second.use(conn);
    return std::string();
  }

  // release/<name> -- notify server that device can be closed
  if (act == "release") {
    if (arg=="")
      throw Err() << "device name expected: " << url;
    auto lk = get_sh_lock();
    if (devices.count(arg) == 0)
      throw Err() << "unknown device: " << arg;
    devices.find(arg)->second.release(conn);
    return std::string();
  }

  // info/<name> -- print device <name> information
  if (act == "info") {
    if (arg=="")
      throw Err() << "device name expected: " << url;
    auto lk = get_sh_lock();
    if (devices.count(arg) == 0)
      throw Err() << "unknown device: " << arg;
    return devices.find(arg)->second.print(conn);
  }

  // devices, list -- list all available devices
  if (act == "devices" || act == "list") {
    if (arg!="")
      throw Err() << "unexpected argument: " << url;
    std::string ret;
    for (auto const & d:devices)
      ret += d.first + "\n";
    return ret;
  }

  // ping -- do nothing
  if (act == "ping"){
    return std::string();
  }

  // print current time (unix seconds with ms precision)
  if (act == "get_time"){
    if (arg!="")
      throw Err() << "unexpected argument: " << url;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    std::ostringstream s;
    s << tv.tv_sec << "." << std::setfill('0') << std::setw(6) << tv.tv_usec;
    return s.str();
  }

  throw Err() << "unknown action: " << act;
}

/*************************************************/
void
DevManager::read_conf(const std::string & file){
  std::map<std::string, Device> ret;
  int line_num[2] = {0,0};
  std::ifstream ff(file);
  if (!ff.good()) throw Err()
    << "can't open configuration: " << file;

  Log(1) << "Reading configuration file: " << file;

  try {
    while (1){
      std::vector<std::string> vs = read_words(ff, line_num, false);
      if (vs.size() < 1) break;
      if (vs.size() < 2) throw Err()
        << "expected: <device name> <driver name> [-<parameter> <value>]";
      std::string dev = vs[0];
      std::string drv = vs[1];
      std::vector<std::string> args(vs.begin()+2, vs.end());
      if ((vs.size()-2)%2 != 0) throw Err()
        << "even-size list of [-<parameter> <value>] pairs expected";
      Opt opt;
      for (int i=2; i+1<vs.size(); i+=2){
        if (vs[i].size()<2 || vs[i][0]!='-') throw Err()
          << "parameter name should be prefixed with \"-\" "
          << "and contain at least one character: " << vs[i];
        opt.put(vs[i].substr(1), vs[i+1]);
      }

      // do not allow empty devices
      if (dev == "") throw Err() << "empty device";

      // do not allow SERVER device
      if (dev == SRVDEV) throw Err() << "can't redefine " << SRVDEV << " device";

      // does this device exists
      if (ret.count(dev)>0) throw Err()
        << "duplicated device name: " << dev;

      // add device information
      ret.emplace(dev, Device(dev,drv,opt));

    }
  } catch (Err e){
    throw Err() << "bad configuration file "
                << file << " at line " << line_num[0] << ": " << e.str();
  }

  Log(1) << ret.size() << " devices configured";

  auto lk = get_lock();
  devices = ret; // apply the configuration only if no errors have found.
}

