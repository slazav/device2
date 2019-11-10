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

struct Device : public std::shared_ptr<Driver>{

  // Constructor
  Device(const std::string & drv_name,
          const Opt & drv_args):
    std::shared_ptr<Driver>(Driver::create(drv_name, drv_args)) {}
};

/*************************************************/

struct DevManager {

  // mutext for dev_map locks (avoid races when opening devices)
  Lock main_lock;

  // All devices (from configuration file):
  std::map<std::string, Device> devices;

  /******/

  DevManager();

  // process a request form HTTP server (given as 'URL').
  std::string run(const std::string & url);

  // Read configuration file, return DevName->Device map.
  // Throw exception on errors.
  // This is a static function. I plan to use it in the constructor,
  // and make an additional command to rereading configuration.
  void read_conf(const std::string & file);

  // parse url, return three-component vector (name, command, data)
  static std::vector<std::string> parse_url(const std::string & url);

};

#endif