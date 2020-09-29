#ifndef DRV_SERIAL_ASM340_H
#define DRV_SERIAL_ASM340_H

/*************************************************/
/*
 * Driver for Pfeiffer Adixen ASM340 leak detector.
 *

Options:

  Serial port setup.

  -dev <str>     -- Serial device filename (e.g. /dev/ttyUSB0)
                    Required.

  -timeout <v>   -- Read timeout, seconds [0 .. 25.5]
                    Default: 5.0

  -sfc (0|1)     -- Software flow control (both 0 and 1 should work).
                    Default: 1

  -errpref <str> -- error prefix (default "ASM340: ")

  -idn <str>     -- override output of *idn? command
                    Default: "Adixen ASM340 leak detector"
*/

#include "drv_serial.h"

class Driver_serial_asm340: public Driver_serial {
  Opt add_opts(const Opt & opts){
    opts.check_unknown({"dev", "timeout", "sfc", "errpref", "idn"});
    Opt o(opts);
    o.put("speed",  9600);  // baud rate
    o.put("parity", "8N1"); // character size, parity, stop bit
    o.put("cread",  1);     // always set cread=1
    o.put("clocal", 1);     // always set clocal=1
    o.put("vmin",   0); // should be set with timeout
    o.put("ndelay", 0); // should be set with timeout
    o.put("icrnl",  0); // do not convert CR->NL on input
    o.put("raw", 1);     // raw mode!
    o.put("delay", 0.1); // 100ms delay after write
    o.put("opost", 0);   // no output postprocessing
    o.put("ack_ch", 0x06);  // trim ack char
    o.put("nack_ch", 0x15); // trim nack char, return communication errors
    o.put("add_ch",  0x0D); // add CR to each sent message
    o.put("trim_ch", 0x0D); // trim CR from each recieved message
    // set defaults (only it no values are set by user)
    o.put_missing("timeout", 5.0);
    o.put_missing("sfc", 1);
    o.put_missing("errpref", "ASM340: ");
    o.put_missing("idn", "Adixen ASM340 leak detector");
    return o;
  }
public:
  Driver_serial_asm340(const Opt & opts): Driver_serial(add_opts(opts)){}
};

#endif
