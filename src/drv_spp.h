#ifndef DRV_SPP_H
#define DRV_SPP_H

#include "drv.h"
#include "iofilter/iofilter.h"

class Driver_spp: public Driver {
  std::shared_ptr<IOFilter> flt;
  std::string prog;
  char ch; // protocol special character
  int ver; // protocol version
  double open_timeout, read_timeout, close_timeout;
  std::string errpref; // error prefix
  std::string idn;

  // read SPP message until #OK or #Error line
  std::string read_spp(double timeout = -1);

public:

  Driver_spp(const Opt & opts);
  ~Driver_spp();

  std::string read();
  void write(const std::string & msg);
  std::string ask(const std::string & msg) override;
};

#endif
