#ifndef DRV_USBTMC_H
#define DRV_USBTMC_H

#include "drv.h"

/*************************************************/
/* Driver `usbtmc` -- using usbtmc kernel module

This driver supports devices connected via usbtmc kernel driver. It
should work with all usual Agilent/Keysight devices connected via USB.
Read timeout is set internally in the driver (5s by default?) but can be
modified by -timeout parameter.

Driver reads answer from the device only if there is a question mark '?'
in the message.

Parameters:

* `-dev <v>`      -- Serial device filename (e.g. /dev/usbtmc0)
                     Required.

* `-timeout <v>`  -- Timeout for reading, seconds
                     Default: do not change

* `-errpref <v>`  -- Prefix for error messages.
                     Default "usbtmc: ".

* `-idn <v>`      -- Override output of *idn? command.
                     Default: do not override.

* `-add_str <v>`  -- Add string to each message sent to the device.
                     Default: "\n"

* `-trim_str <v>` -- Remove string from the end of recieved messages.
                     Default: "\n"

*/

class Driver_usbtmc: public Driver {
  int fd;              // file descriptor
  std::string errpref; // error prefix
  std::string idn;
  std::string add,trim;

public:
  Driver_usbtmc(const Opt & opts);
  ~Driver_usbtmc();

  std::string read() override;
  void write(const std::string & msg) override;
  std::string ask(const std::string & msg) override;
};

#endif
