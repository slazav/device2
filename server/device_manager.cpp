#include <iostream>
#include <fstream>

#include "err/err.h"
#include "read_words/read_words.h"
#include "locks.h"
#include "drivers.h"
#include "device_manager.h"

#define PAGE "<html><head><title>libmicrohttpd demo</title>"\
             "</head><body>libmicrohttpd demo</body></html>"

/*************************************************/
DeviceManager::DeviceManager(){
  // Initalize mutex.
  MHD_mutex_init_(&mutex);

  // Fill drv_info:
  drv_info.emplace("dummy", new Driver_dummy);
}

/*************************************************/
DeviceManager::~DeviceManager(){
  MHD_mutex_destroy_(&mutex);
}

/*************************************************/
std::vector<std::string>
DeviceManager::parse_url(const std::string & url){
  std::vector<std::string> ret(3);
  size_t p1(0), i(0);
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
std::string
DeviceManager::run(const std::string & url){

  auto vs = parse_url(url);
  std::string dev = vs[0];
  std::string cmd = vs[1];
  std::string arg = vs[2];

  // Do we know this device?
  if (dev_info.count(dev) == 0)
    throw Err() << "unknown device:" << dev;
  auto & info = dev_info.find(dev)->second;

  // Do we know this driver? (this should be already
  // checked in read_conf).
  if (drv_info.count(info.drv_name) == 0)
    throw Err() << "unknown driver:" << dev;
  auto & drv = drv_info.find(info.drv_name)->second;

  // Open device if needed
  if (dev_map.count(dev) == 0){
    MHD_mutex_lock_(&mutex);
    dev_map.emplace(dev, drv.get());
    MHD_mutex_unlock_(&mutex);
  }

  // TESTING
  std::cerr << "recieved request: " << url << "\n";
  MHD_mutex_lock_(&mutex);


  std::cerr << "start processing request: " << url << "\n";
  sleep(5);
  std::cerr << "stop processing request: " << url << "\n";

  MHD_mutex_unlock_(&mutex);
  std::cerr << "sending answer: " << url << "\n";
  return PAGE;
}

/*************************************************/
void
DeviceManager::read_conf(const std::string & file){
  std::map<std::string, DevInfo> ret;
  int line_num[2] = {0,0};
  std::ifstream ff(file);
  if (!ff.good()) throw Err()
    << "dev_server: can't open configuration: " << file;

  try {
    while (1){
      std::vector<std::string> vs = read_words(ff, line_num, true);
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

      // do we know the driver?
      if (drv_info.count(drv) == 0) throw Err()
        << "unknown driver: " << drv;

      // does this device exists
      if (ret.count(dev)>0) throw Err()
        << "duplicated device name: " << dev;

      // add device information
      ret.emplace(dev, DevInfo(drv, opt));

    }
  } catch (Err e){
    throw Err() << "dev_server: bad configuration file "
                << file << " at line " << line_num[0] << ": " << e.str();
  }
  MHD_mutex_lock_(&mutex);
  dev_info = ret; // replace configuration only if no errors found.
  MHD_mutex_unlock_(&mutex);
}

