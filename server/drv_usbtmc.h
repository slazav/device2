#ifndef DRV_USBTMC_H
#define DRV_USBTMC_H

#include "drv.h"

/*************************************************/
// usbtmc driver
// This driver supports devices connected via usbtmc kernel driver.
// It should work with all usual Agilent/Keysight devices connected
// via USB. Read timeout is set internally in the driver (5s?)
// but can be changed by -read_timeout option in the configuration file.
// Driver reads answer from the device only if there is a question mark '?'
// in the message.
// Options:
//  -dev -- serial device filename (e.g. /dev/usbtmc0)
//  -timeout -- timeout for reading, seconds (default: do not change)
//  -errpref -- error prefix (default "usbtmc: ")
//  -idn     -- override output of *idn? command (default: do not override)


class Driver_usbtmc: public Driver {
  int fd;              // file descriptor
  std::string errpref; // error prefix
  std::string idn;

public:
  Driver_usbtmc(const Opt & opts);
  ~Driver_usbtmc();

  std::string read() override;
  void write(const std::string & msg) override;
  std::string ask(const std::string & msg) override;
};

#endif
