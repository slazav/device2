#ifndef DRV_H
#define DRV_H

#include <string>
#include <memory>
#include "opt/opt.h"

/*************************************************/
// base class

class Driver {
protected:
  Opt opts;
  Driver(const Opt & opts): opts(opts) {}

public:
  // Create and return a device driver (static function).
  // Parameter `args` contains parameters from the configuration file.
  // They describe how the driver can find the device.
  static std::shared_ptr<Driver> create(
    const std::string & name, const Opt & args);

  // Open device.
  virtual void open() = 0;

  // Close device.
  virtual void close() = 0;

  // Send message to the device, get answer
  virtual std::string ask(const std::string & msg) = 0;

};

#endif
