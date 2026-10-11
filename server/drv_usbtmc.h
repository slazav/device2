#ifndef DRV_USBTMC_H
#define DRV_USBTMC_H

#include "drv.h"
#include "drv_utils.h"

class Driver_usbtmc: public Driver {
  int fd;              // file descriptor
  std::string errpref; // error prefix
  std::string idn;
  std::string add,trim;
  bool auto_abort;     // can we use auto_abort feature of usbtmc driver?
  read_cond_t read_cond;
  double delay;

public:
  Driver_usbtmc(const Opt & opts);
  ~Driver_usbtmc();

  std::string read();
  void write(const std::string & msg);
  std::string ask(const std::string & msg) override;
};

#endif
