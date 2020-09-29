#ifndef DRV_SERIAL_H
#define DRV_SERIAL_H

#include <memory>
#include "drv.h"
#include "opt/opt.h"
#include "io_serial.h"

/*************************************************/
// Serial driver
// Works with old RS-232 SCPI devices such as Keysight/HP 34401.
// Device confiduration is needed: RS-232 9600 8N.
//  -dev -- serial device name

class Driver_serial: public Driver {
  Opt opts;
  std::shared_ptr<IOSerial> serial;

public:

  Driver_serial(const Opt & opts);
  void open() override;
  void close() override;
  std::string ask(const std::string & msg) override;
};

#endif
