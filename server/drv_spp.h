#ifndef DRV_SPP_H
#define DRV_SPP_H

#include "drv.h"
#include "iofilter/iofilter.h"

/*************************************************/
/* driver `spp` -- programs following "Simple Pipe protocol"

This driver implements "Simple pipe protocol" for communicating with
programs using stdin/stdout unix pipes. The protocol is described in
https://github.com/slazav/tcl-device (see Readme.md) It is used in a few
of my projects (pico_rec, graphene), and in `device_c` client program.

Parameters:

* `-prog`          -- program name

* `-open_timeout`  -- timeout for opening

* `-read_timeout`  -- timeout for reading

* `-errpref` -- error prefix (default "spp: ")

* `-idn`     -- override output of *idn? command (default: do not override)

*/

class Driver_spp: public Driver {
  std::shared_ptr<IOFilter> flt;
  std::string prog;
  char ch; // protocol special character
  int ver; // protocol version
  double open_timeout, read_timeout;
  std::string errpref; // error prefix
  std::string idn;

  // read SPP message until #OK or #Error line
  std::string read_spp(double timeout = -1);

public:

  Driver_spp(const Opt & opts);
  ~Driver_spp();

  std::string read() override;
  void write(const std::string & msg) override;
  std::string ask(const std::string & msg) override;
};

#endif
