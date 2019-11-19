#include <iostream>
#include <fstream>

#include "err/err.h"
#include "read_words/read_words.h"
#include "locks.h"
#include "drivers.h"
#include "dev_manager.h"

/*************************************************/
DevManager::DevManager(std::ostream & log, int verb):
   log(log), verb(verb), Lock("manager"){ }

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
  if (verb>1) log << "Connection #" << conn << ": open\n";
}

void
DevManager::conn_close(const uint64_t conn){
  // go through all devices, close ones which are not needed
  if (verb>1) log << "Connection #" << conn << ": close\n";
  for (auto & d:devices){
    if (d.second.close(conn) && verb>1)
      log << d.first << ": close\n";
  }
}

/*************************************************/
std::string
DevManager::run(const std::string & url, const uint64_t conn){
  auto vs = parse_url(url);
  std::string dev = vs[0];
  std::string cmd = vs[1];
  std::string arg = vs[2];

  if (verb>1) log << "Connection #" << conn << ": get request: " << url << "\n";

  try {
    // Do we know this device?
    if (dev == "") throw Err() << "empty device";

    if (devices.count(dev) == 0)
      throw Err() << "unknown device: " << dev;
  }
  catch (Err e){
    if (verb>1)
      log << "Connection #" << conn << ": error: " << e.str() << "\n";
    throw e;
  }

  try {
    Device & d = devices.find(dev)->second;
    if (d.open(conn) && verb>1)
      log << dev << ": open\n";

    if (verb>2)
      log << dev << ": #" << conn << " >> " << cmd << ": " << arg << "\n";
    auto ret = d.cmd(cmd, arg);
    if (verb>2)
      log << dev << ": #" << conn << " << answer: " << ret << "\n";

    return ret;
  }
  catch (Err e){
    if (verb>1)
      log << dev << ": #" << conn << " << error: " << e.str() << "\n";
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
  if (verb>0) log << "Reading configuration file: " << file << "\n";

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
  lock();
  if (verb>0) log << ret.size() << " devices configured\n";
  devices = ret; // apply the configuration only if no errors have found.
  unlock();
}

