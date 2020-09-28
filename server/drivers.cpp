#include "drivers.h"

std::shared_ptr<Driver>
Driver::create(const std::string & name, const Opt & args){
  if (name == "test")    return std::shared_ptr<Driver>(new Driver_test(args));
  if (name == "spp")     return std::shared_ptr<Driver>(new Driver_spp(args));
  if (name == "usbtmc")  return std::shared_ptr<Driver>(new Driver_usbtmc(args));
  if (name == "tenma_ps")  return std::shared_ptr<Driver>(new Driver_tenma_ps(args));
  throw Err() << "unknown driver: " << name;
}
