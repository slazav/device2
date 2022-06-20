#ifndef DRV_SERIAL_TENMA_PS_H
#define DRV_SERIAL_TENMA_PS_H

/*************************************************/
/*
 * Driver `serial_tenma_ps` -- Korad/Velleman/Tenma power supplies
 *

Options:

* `-dev <v>`      -- Serial device filename (e.g. /dev/ttyACM0)
                    Required.

* `-timeout <v>`  -- Read timeout, seconds [0 .. 25.5]
                    Default: 5.0

* `-errpref <v>` -- Prefix for error messages.
                    Default "TenmaPS: "

* `-idn <v>`     -- Override output of *idn? command.
                    Default: do not override.

*/

#include "drv_serial.h"

class Driver_serial_tenma_ps: public Driver_serial {
  Opt add_opts(const Opt & opts){
    opts.check_unknown({"dev", "timeout", "errpref", "idn"});
    Opt o(opts);
    o.put("speed",  9600);  // baud rate
    o.put("parity", "8N1"); // character size, parity, stop bit
    o.put("cread",  1);     // always set cread=1
    o.put("clocal", 1);     // always set clocal=1
    o.put("vmin",   0); // should be set with timeout
    o.put("ndelay", 0); // should be set with timeout
    o.put("sfc",    0); // important, with sfc=1 Tenma can't send 0 status byte!
    o.put("raw",    1); // raw mode!
    o.put("delay",0.1); // 100ms delay after write
    o.put("opost",  1); // no output postprocessing, for case conversion
    o.put("olcuc",  1); // convert messages to upper case
    o.put("read_cond",  "qmark"); // read answers only if ? was in the message

    // set defaults (only it no values are set by user)
    o.put_missing("timeout", 5.0);
    o.put_missing("errpref", "TenmaPS: ");

    return o;
  }
public:
  Driver_serial_tenma_ps(const Opt & opts): Driver_serial(add_opts(opts)){}
};

#endif
