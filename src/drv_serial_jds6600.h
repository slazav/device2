#ifndef DRV_SERIAL_JDS6600_H
#define DRV_SERIAL_JDS6600_H

#include "drv_serial.h"

class Driver_serial_jds6600: public Driver_serial {
  Opt add_opts(const Opt & opts){
    opts.check_unknown({"dev", "timeout", "errpref", "idn"});
    Opt o(opts);
    o.put("speed", 115200); // baud rate
    o.put("parity", "8N1"); // character size, parity, stop bit
    o.put("cread",  1);     // always set cread=1
    o.put("clocal", 1);     // always set clocal=1
    o.put("sfc",    0);
    o.put("raw",    1);
    o.put("delay",0.1); // 100ms delay after write
    o.put("onlcr",  1); // convert \n -> \n\r on writing
    o.put("igncr",  1); // ignore \r on reading
    o.put("add_str",  "\n");
    o.put("trim_str", "\n");
    o.put("read_cond",  "always"); // always read answers

    // set defaults (only it no values are set by user)
    o.put_missing("timeout", 5.0);
    o.put_missing("errpref", "JDS6600: ");
    o.put_missing("idn", "JDS6600");

    return o;
  }
public:
  Driver_serial_jds6600(const Opt & opts): Driver_serial(add_opts(opts)){}
};

#endif
