#ifndef DRV_NET_GPIB_PROLOGIX_H
#define DRV_NET_GPIB_PROLOGIX_H

#include <memory>
#include "drv_net.h"
#include "opt/opt.h"

class Driver_net_gpib_prologix: public Driver_net {
  std::string gpib_addr; // gpib address

public:

  Driver_net_gpib_prologix(const Opt & opts):
    Driver_net(set_opt(opts)) {}

  std::string read(){
    sel_device();
    return Driver_net::read();
  }

  void write(const std::string & msg){
    sel_device();
    Driver_net::write(msg);
  }

  // Select gpib device.
  // Get address, change it if needed.
  void sel_device() {
    std::string acmd("++addr");
    Driver_net::write(acmd);
    auto a = Driver_net::read();
    if (a != gpib_addr)
      Driver_net::write(acmd + " " + gpib_addr);
  }

  // set options for net driver
  Opt set_opt(const Opt & opts){
    // Options "add_str", "trim_str" are not needed, we want to use
    // default values, or set them here.
    opts.check_unknown({"addr","port","timeout","bufsize","errpref","idn","gpib_addr"});

    // Copy options to modify them for Driver_net.
    // Only override default port setting.
    Opt o(opts);
    o.put_missing("port", "1234");

    // Process options specific for gpib_prologix driver.
    // (gpib address on the device we want to access)
    gpib_addr = opts.get("gpib_addr", "");
    o.erase("gpib_addr");
    if (gpib_addr == "") throw Err() << errpref
      << "Parameter -gpib_addr is empty or missing";

    return o;
 }

};

#endif
