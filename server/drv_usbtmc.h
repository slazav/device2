#ifndef DRV_USBTMC_H
#define DRV_USBTMC_H

#include "drv.h"

//#include <string>
//#include <map>
//#include <memory>
//#include <cstring>

//#include "log/log.h"
//#include "opt/opt.h"
//#include "iofilter/iofilter.h"

//#include "tmc.h"
//#include <unistd.h>
//#include <sys/types.h>
//#include <sys/stat.h>
//#include <sys/ioctl.h>
//#include <fcntl.h>

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
//  -read_timeout  -- timeout for reading, seconds (default 0 - do not change)

class Driver_usbtmc: public Driver {
  std::string dev; // serial device name
  int fd;          // file descriptor
  double read_timeout; // read timeout, s

public:
  Driver_usbtmc(const Opt & opts);
  void open() override;
  void close() override;
  std::string ask(const std::string & msg) override;
};

#endif
