#ifndef DRV_SPP_H
#define DRV_SPP_H

#include "drv.h"
#include "iofilter/iofilter.h"

/*************************************************/
// SPP driver
// This driver implements "Simple pipe prococol" for comminicating with
// programs using stdin/stdout unix pipes.
// Used in a few of my projects (pico_rec, graphene),
// described in https://github.com/slazav/tcl-device (see Readme.md)
// Options:
//  -prog          -- program name
//  -open_timeout  -- timeout for opening
//  -read_timeout  -- timeout for reading

class Driver_spp: public Driver {
  std::shared_ptr<IOFilter> flt;
  std::string prog;
  char ch; // protocol special character
  int ver; // protocol version
  double open_timeout, read_timeout;

  // read SPP message until #OK or #Error line
  std::string read_spp(double timeout = -1);

public:

  Driver_spp(const Opt & opts);
  void open() override;
  void close() override;
  std::string ask(const std::string & msg) override;
};

#endif
