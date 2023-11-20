#ifndef DRV_VXI_H
#define DRV_VXI_H
#ifdef USE_VXI

#include <memory>
#include "drv.h"
#include "drv_utils.h"
#include "opt/opt.h"

#include "vxi/vxi.h"

/*************************************************/
/*
 * Driver "vxi" -- VXI-11 devices
 *

Driver reads answer from the device only if there is a question mark '?'
in the message.

Parameters:

* `-addr <N>`      -- IP/hostname.
                      Required.

* `-name <N>`      -- Instrument name ("instr0", "gpib0,10", etc.).
                      Required.

* `-rpc_timeout <float>`  -- I/O timeout in seconds.
                             Default 2.0

* `-io_timeout <float>`   -- I/O timeout in seconds.
                             Default 5.0

* `-lock_timeout <float>` -- I/O timeout in seconds.
                             Default 2.0

* `-errpref <str>` -- Prefix for error messages.
                      Default: "vxi: "

* `-idn <str>`     -- Override output of *idn? command.
                      Default: empty string, do not override.

* `-read_cond <v>` -- When do we need to read answer from a command:
                      always, never, qmark (if there is a question mark in the message),
                      qmark1w (question mark in the first word). Default: qmark1w.

* `-add_str <v>`   -- Add string to each message sent to the device.
                      Default: "\n"

* `-trim_str <v>`  -- Remove string from the end of received messages.
                      Default: "\n"

* `-delay <v>`     -- Delay after write command, s.
                      Default: 0

*/

class Driver_vxi: public Driver {
protected:
  int dh; // GPIB device handler
  std::string errpref,idn;
  std::string add,trim;
  read_cond_t read_cond;
  double delay;
  std::shared_ptr<VXI> dev;

public:

  Driver_vxi(const Opt & opts);

  std::string read() override;
  void write(const std::string & msg) override;
  std::string ask(const std::string & msg) override;
};

#endif
#endif
