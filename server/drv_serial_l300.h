#ifndef DRV_SERIAL_L300_H
#define DRV_SERIAL_L300_H

#include "drv_serial.h"

class Driver_serial_l300: public Driver_serial {
  Opt add_opts(const Opt & opts){
    opts.check_unknown({"dev", "timeout", "sfc", "errpref", "idn"});
    Opt o(opts);
    o.put("speed",  19200);  // baud rate
    o.put("parity", "8N1"); // character size, parity, stop bit
    o.put("cread",  1);     // always set cread=1
    o.put("clocal", 1);     // always set clocal=1
    o.put("icrnl",  0);  // do not convert CR->NL on input
    o.put("raw", 1);     // raw mode!
    o.put("sfc", 1);     // software flow control
    o.put("crtscts", 0); // no hardware handshake
    o.put("add_str", "\r");   // add CR to each sent message
    o.put("trim_str","\r");   // trim CR from each received message
    o.put("read_cond",  "always");
    // set defaults (only it no values are set by user)
    o.put_missing("timeout", 5.0);
    o.put_missing("errpref", "L300: ");
    o.put_missing("idn", "PhoeniXL300");
    return o;
  }
public:
  Driver_serial_l300(const Opt & opts): Driver_serial(add_opts(opts)){}
};

#endif
