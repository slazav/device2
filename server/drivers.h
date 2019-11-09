#ifndef DRIVERS_H
#define DRIVERS_H

#include <cstring>

/*************************************************/
// base class

struct Driver {
  // driver can lock some resource (unix device name,
  // ip address, etc.)
  // Should be valid when driver is opened.
//  virtual void get_lock_name() = 0;

  virtual void open(const std::string & pars) = 0;
  virtual void close() = 0;
  virtual std::string command(const std::string & cmd) = 0;
};
/*************************************************/
// dummy driver

struct Driver_dummy: Driver {
  void open(const std::string & pars) override {}
  void close() override {}
  std::string command(const std::string & cmd) override {return cmd;};
};

#endif