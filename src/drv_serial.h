#ifndef DRV_SERIAL_H
#define DRV_SERIAL_H

#include <memory>
#include "drv.h"
#include "drv_utils.h"
#include "opt/opt.h"
#include <string>
#include "opt/opt.h"

class Driver_serial: public Driver {
protected:
  int fd; // file descriptor
  std::string errpref, idn;
  std::string ack,nack,add,trim;
  read_cond_t read_cond;
  double delay;
  bool flush_on_err;
  bool spp;

public:

  Driver_serial(const Opt & opts);
  ~Driver_serial();

  std::string read();
  void write(const std::string & msg);
  std::string ask(const std::string & msg) override;
};

#endif
