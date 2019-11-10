#include "drivers.h"

std::shared_ptr<Driver>
Driver::create(const std::string & name, const Opt & args){
  if (name == "dummy") return std::shared_ptr<Driver>(new Driver_dummy);
  throw Err() << "unknown driver: " << name;
}
