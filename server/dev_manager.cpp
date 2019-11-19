#include <iostream>
#include <fstream>

#include "err/err.h"
#include "log/log.h"
#include "read_words/read_words.h"
#include "locks.h"
#include "drivers.h"
#include "dev_manager.h"

/*************************************************/
DevManager::DevManager(): Lock("manager"){ }

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
  Log(2) << "Connection #" << conn << ": open";
}

void
DevManager::conn_close(const uint64_t conn){
  // go through all devices, close ones which are not needed
  Log(2) << "Connection #" << conn << ": close";
  for (auto & d:devices){
    if (d.second.close(conn))
      Log(2) << d.first << ": close";
  }
}

/*************************************************/
std::string
DevManager::run(const std::string & url, const uint64_t conn){
  auto vs = parse_url(url);
  std::string dev = vs[0];
  std::string cmd = vs[1];
  std::string arg = vs[2];

  Log(2) << "Connection #" << conn << ": get request: " << url;

  try {
    // Do we know this device?
    if (dev == "") throw Err() << "empty device";

    if (devices.count(dev) == 0)
      throw Err() << "unknown device: " << dev;
  }
  catch (Err e){
    Log(2) << "Connection #" << conn << ": error: " << e.str();
    throw e;
  }

  try {
    Device & d = devices.find(dev)->second;
    if (d.open(conn)) Log(2) << dev << ": open";

    Log(3) << dev << ": #" << conn << " >> " << cmd << ": " << arg;
    auto ret = d.cmd(cmd, arg);
    Log(3) << dev << ": #" << conn << " << answer: " << ret;

    return ret;
  }
  catch (Err e){
    Log(2) << dev << ": #" << conn << " << error: " << e.str();
    throw e;
  }
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
        opt.put(vs[i], vs[i+1]);
      }

      // do not allow empty devices
      if (dev == "") throw Err() << "empty device";

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

  lock();
  devices = ret; // apply the configuration only if no errors have found.
  unlock();
}

