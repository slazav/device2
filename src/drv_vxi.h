#ifndef DRV_VXI_H
#define DRV_VXI_H
#ifdef USE_VXI

#include <memory>
#include "drv.h"
#include "drv_utils.h"
#include "opt/opt.h"

#include "vxi/vxi.h"

class Driver_vxi: public Driver {
protected:
  int dh; // GPIB device handler
  std::string errpref,idn;
  std::string add,trim;
  read_cond_t read_cond;
  double delay;
  std::shared_ptr<VXI> dev;

public:

  Driver_vxi(const Opt & opts);

  std::string read();
  void write(const std::string & msg);
  std::string ask(const std::string & msg) override;
};

#endif
#endif
