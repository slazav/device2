#include "drivers.h"

std::shared_ptr<Driver>
Driver::create(const std::string & name, const Opt & args){
  if (name == "TEST") return std::shared_ptr<Driver>(new Driver_test);
  throw Err() << "unknown driver: " << name;
}
