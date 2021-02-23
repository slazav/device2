#ifndef DRV_GPIB_H
#define DRV_GPIB_H

#include <gpib/ib.h>
#include "drv.h"
#include "drv_utils.h"
#include "opt/opt.h"

/*************************************************/
/*
 * Driver "gpib" -- GPIB devices using linux-gpib library
 *

Driver reads answer from the device only if there is a question mark '?'
in the message.

Parameters:

* `-addr <N>`      -- GPIB address.
                      Required.

* `-board <N>`     -- GPIB board number.
                      Default: 0.

* `-timeout <str>` -- I/O timeout: none, 10us, 30us, 100us ... 300s, 1000s
                      Default 10s.

* `-open_timeout <str>` -- Open timeout, same values as for -read_timeout.
                      Default 3s.

* `-eot (1|0)`     -- Enable/disable assertion of EOI line with last data byte
                      Default: do not change

* `-eos_mode <v>`  -- Set end-of-string mode (none, read, write, bin)
                      Only works with -eos option. Default: none.

* `-eos (0..255)`  -- Set end-of-string character.
                      Default: do not change

* `-secondary (1|0)` -- Set secondary GPIB address.

* `-bufsize <N>`   -- Buffer size for reading. Maximum length of read data.
                      Default: 4096

* `-errpref <str>` -- Prefix for error messages.
                      Default: "gpib: "

* `-idn <str>`     -- Override output of *idn? command.
                      Default: empty string, do not override.

* `-read_cond <v>` -- When do we need to read answer from a command:
                      always, never, qmark (if there is a question mark in the message),
                      qmark1w (question mark in the first word). Default: qmark1w.

* `-add_str <v>`   -- Add string to each message sent to the device.
                      Default: "\n"

* `-trim_str <v>`  -- Remove string from the end of received messages.
                      Default: "\n"

*/

class Driver_gpib: public Driver {
protected:
  int dh; // GPIB device handler
  size_t bufsize;
  std::string errpref,idn;
  std::string add,trim;
  read_cond_t read_cond;

  // convert timeout
  int get_timeout(const std::string & s);

public:

  Driver_gpib(const Opt & opts);
  ~Driver_gpib();

  std::string read() override;
  void write(const std::string & msg) override;
  std::string ask(const std::string & msg) override;
};

#endif
