#ifndef DRV_TEST_H
#define DRV_TEST_H

#include "drv.h"
#include "err/err.h"

/*************************************************/
// Test driver
// Configuration options: none
// Actions:
//   ask - just return the action argument

class Driver_test: public Driver {
  bool opened;
public:
  Driver_test(const Opt & opts): Driver(opts) {}
  void open()  override {opened=true;}
  void close() override {opened=false;}
  std::string ask(const std::string & msg) override {
    if (!opened) throw Err() << "Test: device if closed";
    return msg;
  };
};


#endif
