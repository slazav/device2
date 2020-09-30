#ifndef DRV_NET_GPIB_PROLOGIX_H
#define DRV_NET_GPIB_PROLOGIX_H

#include <memory>
#include "drv_net.h"
#include "opt/opt.h"

/*************************************************/
/*
 * Access to device via Prologix gpib2eth converter.
 *

Not tested!

Parameters:

* `-addr <v>`      -- Network address or IP.
                      Required.
* `-port <v>`      -- Port number.
                      Default: "5025" (lxi raw protocol).
* `-timeout <v>`   -- Read timeout, seconds. No timeout if <=0.
                      Default 5.0.
* `-bufsize <v>`   -- Buffer size for reading. Maximum length of read data.
                      Default: 4096
* `-errpref <v>`   -- Prefix for error messages.
                      Default: "IOSerial: "
* `-idn <v>`       -- Override output of *idn? command.
                      Default: empty string, do not override.
* `-addr <v>`      -- GPIB address

*/

class Driver_net_gpib_prologix: public Driver_net {
  std::string addr; // gpib address

public:

  Driver_net_gpib_prologix(const Opt & opts):
    Driver_net(set_opt(opts)) {}

  std::string read() override {
    sel_device();
    return Driver_net::read();
  }

  void write(const std::string & msg) override {
    sel_device();
    Driver_net::write(msg);
  }

  // Select gpib device.
  // Get address, change it if needed.
  void sel_device() {
    std::string acmd("++addr");
    Driver_net::write(acmd);
    auto a = Driver_net::read();
    if (a != addr)
      Driver_net::write(acmd + " " + addr);
  }

  // set options for net driver
  Opt set_opt(const Opt & opts){
    // Options "add_str", "trim_str" are not needed, we want to use
    // default values, or set them here.
    opts.check_unknown({"addr","port","timeout","bufsize","errpref","idn"});

    // Copy options to modify them for Driver_net.
    // Only override default port setting.
    Opt o(opts);
    o.put_missing("port", "1234");

    // Process options specific for gpib_prologix driver.
    // (gpib address on the device we want to access)
    addr = opts.get("addr", "");
    o.erase("addr");
    if ("addr" == "") throw Err() << errpref
      << "Parameter -addr is empty or missing";

    return o;
 }

};

#endif
