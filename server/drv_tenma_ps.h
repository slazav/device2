#ifndef DRV_TENMA_PS_H
#define DRV_TENMA_PS_H

#include "drv.h"
#include <algorithm>

/*************************************************/
// tenma_ps driver
// This driver works with Korad/Velleman/Tenma power supplies.
// It is a simple communication via a serial device with some
// specific delays and without newline characters.
// Options:
//  -dev -- serial device filename (e.g. /dev/ttyACM0)
//
// See:
// https://sigrok.org/wiki/Korad_KAxxxxP_series
// https://sigrok.org/gitweb/?p=libsigrok.git (src/hardware /korad-kaxxxxp/)

class Driver_tenma_ps: public Driver {
  std::string dev; // serial device name
  int fd;          // file descriptor
  int del;         // delay between write and read, us

public:

  Driver_tenma_ps(const Opt & opts);
  void open() override;
  void close() override;
  std::string ask(const std::string & msg) override;
};

#endif
