#ifndef DRV_SERIAL_ET_H
#define DRV_SERIAL_ET_H

/*************************************************/
/*
 * Driver `serial_et` -- EastTester devices
 *

Options:

* `-dev <v>`      -- Serial device filename (e.g. /dev/ttyACM0)
                    Required.

* `-timeout <v>`  -- Read timeout, seconds [0 .. 25.5]
                    Default: 2.0

* `-errpref <v>` -- Prefix for error messages.
                    Default "EastTester: "

* `-idn <v>`     -- Override output of *idn? command.
                    Default: do not override.

*/

#include "drv_serial.h"

class Driver_serial_et: public Driver_serial {
  Opt add_opts(const Opt & opts){
    opts.check_unknown({"dev", "timeout", "errpref", "idn"});
    Opt o(opts);
    o.put("speed",  9600);  // baud rate
    o.put("parity", "8N1"); // character size, parity, stop bit
    o.put("cread",  1);     // always set cread=1
    o.put("clocal", 1);     // always set clocal=1
    o.put("ndelay", 1); // non-blocking mode!
    o.put("sfc",    1); // default software flow control
    o.put("raw",    1); // raw mode!
    o.put("delay",0.1); // 100ms delay after write
    o.put("add_str","\n");    // add newline to all commands
    o.put("trim_str","\n\n"); // trim two newlines from all commands
    o.put("read_cond",  "always"); // always try to read answer (in non-blocking mode)

    // set defaults (only it no values are set by user)
    o.put_missing("timeout", 2.0);
    o.put_missing("errpref", "EastTester: ");

    return o;
  }
public:
  Driver_serial_et(const Opt & opts): Driver_serial(add_opts(opts)){}
};

#endif
