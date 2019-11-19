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

#include "opt/opt.h"
#include "locks.h"
#include "drivers.h"

/*************************************************/

class Device : std::shared_ptr<Driver>, public Lock{
  // Connections which use the device
  std::set<uint64_t> users;

public:
  // Constructor
  Device( const std::string & dev_name,
          const std::string & drv_name,
          const Opt & drv_args):
    std::shared_ptr<Driver>(Driver::create(drv_name, drv_args)),
    Lock("manager:" + dev_name) {}

  // Reserve device for a connection.
  // Open it if there were no reservations.
  // Return true if the device was opened.
  bool open(const uint64_t conn){
    if (users.count(conn)>0) return false; // device is opened and used by this connection
    lock();
    bool do_open = users.empty();          // device need to be opened
    users.insert(conn);
    if (do_open){
      (*this)->open();
    }
    unlock();
    return do_open;
  }

  // Remove reservation for a connection.
  // Close the device if no reservations left.
  // Return true if the device was closed.
  bool close(const uint64_t conn){
    if (users.count(conn)==0) return false; // device is not used by this connection
    lock();
    users.erase(conn);
    bool do_close = users.empty();
    if (do_close) (*this)->close();
    unlock();
    return do_close;
  }

  std::string cmd(const std::string & cmd, const std::string & arg){
    if (cmd == "ask"){
      lock();
      auto ret = (*this)->cmd(arg);
      unlock();
      return ret;
    }
    throw Err() << "unknown command: " << cmd;
  }

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