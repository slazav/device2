#ifndef DEV_MANGR_H
#define DEV_MANGR_H

#include <microhttpd.h>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <map>
#include <vector>
#include <string>
#include <memory>

#include "opt/opt.h"
#include "locks.h"
#include "drivers.h"

/*************************************************/

struct DevInfo {
  std::string drv_name;
  Opt drv_args;

  DevInfo(const std::string & drv_name,
          const Opt & drv_args):
    drv_name(drv_name), drv_args(drv_args) {}
};

/*************************************************/

struct DevManager {

  // mutext for dev_map locks (avoid races when opening devices)
  Lock main_lock;

  // All known drivers: driver name -> driver class
  std::map<std::string, std::shared_ptr<Driver> > drv_info;

  // All known devices (from configuration file):
  // device name -> driver name, driver parameters.
  std::map<std::string, DevInfo> dev_info;

  // Opened devices.
  std::map<std::string, std::shared_ptr<Driver> > dev_map;

  /******/

  DevManager();

  std::string run(const std::string & url);

  // Read configuration file, return DevName->DevInfo map.
  // Throw exception on errors.
  // This is a static function. I plan to use it in the constructor,
  // and make an additional command to rereading configuration.
  void read_conf(const std::string & file);

  // parse url, return three-component vector (name, command, data)
  static std::vector<std::string> parse_url(const std::string & url);

};

#endif