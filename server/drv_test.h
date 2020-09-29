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
  std::string m;
public:
  Driver_test(const Opt & opts) {}

  std::string read() override {return m;};

  void write(const std::string & msg) override {m = msg;};

  std::string ask(const std::string & msg) override {
    write(msg); return read(); }
};


#endif
