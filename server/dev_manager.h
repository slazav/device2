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

#include "err/err.h"
#include "log/log.h"
#include "opt/opt.h"
#include "device.h"
#include "locks.h"

class DevManager : public Lock {
public:

  // All devices (from configuration file):
  std::map<std::string, Device> devices;

  DevManager();

  // open connection callback:
  void conn_open(const uint64_t conn);

  // close connection callback:
  void conn_close(const uint64_t conn);

  // Process a request from HTTP server.
  // Arguments:
  // - url:  request "URL", device/command/data
  // - conn: connection ID
  std::string run(const std::string & url, const uint64_t conn);

  // Read configuration file, update `devices` map.
  // Throw exception on errors.
  void read_conf(const std::string & file);

  // parse url, return three-component vector (device, command, data)
  static std::vector<std::string> parse_url(const std::string & url);

};

#endif
