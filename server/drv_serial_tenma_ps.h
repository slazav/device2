#ifndef DRV_SERIAL_TENMA_PS_H
#define DRV_SERIAL_TENMA_PS_H

/*************************************************/
/*
 * Driver for Korad/Velleman/Tenma power supplies.
 *

Options:

  Serial port setup.

  -dev <name>   -- Serial device filename (e.g. /dev/ttyACM0)
                   Required.

  -timeout <v>  -- Read timeout, seconds [0 .. 25.5]
                   Default: 5.0

  -errpref      -- error prefix (default "TenmaPS: ")

*/

#include "drv_serial.h"

class Driver_serial_tenma_ps: public Driver_serial {
  Opt add_opts(const Opt & opts){
    opts.check_unknown({"dev", "timeout", "errpref"});
    Opt o(opts);
    o.put("speed",  9600);  // baud rate
    o.put("parity", "8N1"); // character size, parity, stop bit
    o.put("cread",  1);     // always set cread=1
    o.put("clocal", 1);     // always set clocal=1
    o.put("timeout", o.get("timeout", 5.0));     // default timeout
    o.put("vmin",   0); // should be set with timeout
    o.put("ndelay", 0); // should be set with timeout
    o.put("sfc",    1); // default software flow control
    o.put("raw",    1); // raw mode!
    o.put("delay",0.1); // 100ms delay after write
    o.put("opost",  1); // no output postprocessing, for case conversion
    o.put("olcuc",  1); // convert messages to upper case
    o.put("errpref", o.get("errpref", "TenmaPS: ")); // change default error prefix
    return o;
  }
public:
  Driver_serial_tenma_ps(const Opt & opts): Driver_serial(add_opts(opts)){}
};

#endif
