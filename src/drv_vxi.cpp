#ifdef USE_VXI

#include <cstring>
#include "drv_utils.h"
#include "drv_vxi.h"
#include <unistd.h>

/************************************************/

Driver_vxi::Driver_vxi(const Opt & opts) {

  opts.check_unknown({"addr","name",
    "rpc_timeout","io_timeout","lock_timeout"
    "errpref", "idn", "read_cond", "add_str", "trim_str", "delay"});

  //prefix for error messages
  errpref = opts.get("errpref", "vxi: ");

  // address and instrument name (mandatory setting)
  std::string addr = opts.get("addr");
  std::string name = opts.get("name");
  if (addr=="") throw Err() << errpref
    << "Parameter -addr is empty or missing";
  if (name=="") throw Err() << errpref
    << "Parameter -name is empty or missing";

  // timeouts
  double rpc_timeout  = opts.get<double>("rpc_timeout",   2.0);
  double io_timeout   = opts.get<double>("io_timeout",    2.0);
  double lock_timeout = opts.get<double>("lock_timeout", 10.0);

  errpref += addr + ":" + name + ": ";

  dev.reset(new VXI(addr.c_str(), name.c_str(), rpc_timeout));
  dev->set_io_timeout(io_timeout);
  dev->set_lock_timeout(lock_timeout);
  dev->clear();

  add     = opts.get("add_str",  "\n");
  trim    = opts.get("trim_str", "\n");
  delay   = opts.get("delay",    0.0);
  idn     = opts.get("idn", "");
  read_cond = str_to_read_cond(opts.get("read_cond", "qmark1w"));
}

std::string
Driver_vxi::read() {
  std::string ret = dev->read();
  trim_str(ret,trim); // -trim option
  return ret;
}

void
Driver_vxi::write(const std::string & msg) {
  std::string m = msg;
  if (add.size()>0) m+=add;
  dev->write(m.c_str());
  if (delay>0) usleep(delay*1e6);
}

std::string
Driver_vxi::ask(const std::string & msg) {

  if (idn.size() && strcasecmp(msg.c_str(),"*idn?")==0) return idn;

  write(msg);

  // if there is no '?' in the message no answer is needed.
  if (!check_read_cond(msg, read_cond)) return std::string();

  return read();
}

#endif