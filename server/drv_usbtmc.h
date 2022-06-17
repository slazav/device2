#ifndef DRV_USBTMC_H
#define DRV_USBTMC_H

#include "drv.h"
#include "drv_utils.h"

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

* `-timeout <v>`  -- Timeout for reading, seconds.
                     Default: 5.0

*  `-delay <v>`   -- Delay after write command, s.
                     Default: 0.01

* `-errpref <v>`  -- Prefix for error messages.
                     Default "usbtmc: ".

* `-idn <v>`      -- Override output of *idn? command.
                     Default: do not override.

* `-read_cond <v>` -- When do we need to read answer from a command:
                      always, never, qmark (if there is a question mark in the message),
                      qmark1w (question mark in the first word). Default: qmark1w.

* `-add_str <v>`  -- Add string to each message sent to the device.
                     Default: "\n"

* `-trim_str <v>` -- Remove string from the end of received messages.
                     Default: "\n"

*/

class Driver_usbtmc: public Driver {
  int fd;              // file descriptor
  std::string errpref; // error prefix
  std::string idn;
  std::string add,trim;
  bool auto_abort;     // can we use auto_abort feature of usbtmc driver?
  read_cond_t read_cond;
  double delay;

public:
  Driver_usbtmc(const Opt & opts);
  ~Driver_usbtmc();

  std::string read() override;
  void write(const std::string & msg) override;
  std::string ask(const std::string & msg) override;
};

#endif
