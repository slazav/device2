#ifndef DRV_TEST_H
#define DRV_TEST_H

#include "drv.h"
#include "err/err.h"

/*************************************************/
/* Driver `test` -- a dummy driver for tests

Just repeats any message sent to it.
Parameters: none

*/

class Driver_test: public Driver {
  std::string m;
public:
  Driver_test(const Opt & opts) {}

  std::string ask(const std::string & msg) override { return msg; }
};


#endif
