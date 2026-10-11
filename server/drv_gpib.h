#ifndef DRV_GPIB_H
#define DRV_GPIB_H
#ifdef USE_GPIB

#include "drv.h"
#include "drv_utils.h"
#include "opt/opt.h"

class Driver_gpib: public Driver {
protected:
  int dh; // GPIB device handler
  size_t bufsize;
  std::string errpref,idn;
  std::string add,trim;
  read_cond_t read_cond;
  double delay;

  // convert timeout
  int get_timeout(const std::string & s);

public:

  Driver_gpib(const Opt & opts);
  ~Driver_gpib();

  std::string read();
  void write(const std::string & msg);
  std::string ask(const std::string & msg) override;
};

#endif
#endif
