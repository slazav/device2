#ifndef DRV_SERIAL_VS_LD_H
#define DRV_SERIAL_VS_LD_H

/*************************************************/
/*
 * Driver `serial_vs_ld` -- Agilent VS leak detector
 *

Leak detector should be connected with null-modem cable.

Driver is not tested and probably not working!
It was some non-trivial problems with echo.

Parameters:

* `-dev <str>`     -- Serial device filename (e.g. /dev/ttyUSB0)
                      Required.

* `-timeout <v>`   -- Read timeout, seconds [0 .. 25.5]
                      Default: 5.0

* `-errpref <v>`   -- Prefix for error messages.
                      Default "Agilent VS: "

* `-idn <v>`       -- Override output of *idn? command.
                      Default: "Agilent VS leak detector"
*/

#include "drv_serial.h"

class Driver_serial_vs_ld: public Driver_serial {
  Opt add_opts(const Opt & opts){
    opts.check_unknown({"dev", "timeout", "sfc", "errpref", "idn"});
    Opt o(opts);
    o.put("speed",  9600);  // baud rate
    o.put("parity", "8N1"); // character size, parity, stop bit
    o.put("cread",  1);     // always set cread=1
    o.put("clocal", 1);     // always set clocal=1
    o.put("vmin",   0);  // should be set with timeout
    o.put("ndelay", 0);  // should be set with timeout
    o.put("sfc",    0);  // software flow control
    o.put("raw",    1);  // raw mode!
    o.put("delay", 0.1); // 100ms delay after write
    o.put("opost", 0);   // no output postprocessing
    o.put("add_str",  "\n"); // add NL to each sent message
    o.put("trim_str", "\n"); // trim NL from each received message
    o.put("ack_str",  "ok"); // ack sequence
    o.put("nack_str", "#?"); // nack sequence
    // set defaults (only it no values are set by user)
    o.put_missing("timeout", 5.0);
    o.put_missing("errpref", "Agilent VS: ");
    o.put_missing("idn", "Agilent VS leak detector");
    return o;
  }
public:
  Driver_serial_vs_ld(const Opt & opts): Driver_serial(add_opts(opts)){}
};

#endif
