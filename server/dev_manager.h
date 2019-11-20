#ifndef DEV_MANGR_H
#define DEV_MANGR_H

#include <microhttpd.h>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <map>
#include <set>
#include <vector>
#include <string>
#include <memory>

#include "err/err.h"
#include "log/log.h"
#include "opt/opt.h"
#include "locks.h"
#include "drivers.h"

/*************************************************/

class Device : std::shared_ptr<Driver>, public Lock{
  // Connections which use the device
  std::set<uint64_t> users;
  std::string dev;

public:
  // Constructor
  Device( const std::string & dev_name,
          const std::string & drv_name,
          const Opt & drv_args);

  // Reserve device for a connection.
  // Open it if there were no reservations.
  void open(const uint64_t conn);

  // Remove reservation for a connection.
  // Close the device if no reservations left.
  void close(const uint64_t conn);

  // Send command to the device
  std::string cmd(const std::string & cmd, const std::string & arg);

};

/*************************************************/

class DevManager : public Lock {
public:

  // All devices (from configuration file):
  std::map<std::string, Device> devices;

  /******/

  DevManager();

  // open connection callback:
  void conn_open(const uint64_t conn);

  // close connection callback:
  void conn_close(const uint64_t conn);

  // process a request from HTTP server (given as 'URL').
  // conn parameter is the connection ID
  std::string run(const std::string & url, const uint64_t conn);

  // Read configuration file, update `devices` map.
  // Throw exception on errors.
  void read_conf(const std::string & file);

  // parse url, return three-component vector (name, command, data)
  static std::vector<std::string> parse_url(const std::string & url);

};

#endif