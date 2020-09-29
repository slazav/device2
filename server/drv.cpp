#include "drv.h"
#include "drv_test.h"
#include "drv_spp.h"
#include "drv_usbtmc.h"
#include "drv_net.h"
#include "drv_serial.h"
#include "drv_serial_asm340.h"
#include "drv_serial_tenma_ps.h"

std::shared_ptr<Driver>
Driver::create(const std::string & name, const Opt & args){
  if (name == "test")            return std::shared_ptr<Driver>(new Driver_test(args));
  if (name == "spp")             return std::shared_ptr<Driver>(new Driver_spp(args));
  if (name == "usbtmc")          return std::shared_ptr<Driver>(new Driver_usbtmc(args));
  if (name == "serial")          return std::shared_ptr<Driver>(new Driver_serial(args));
  if (name == "serial_asm340")   return std::shared_ptr<Driver>(new Driver_serial_asm340(args));
  if (name == "serial_tenma_ps") return std::shared_ptr<Driver>(new Driver_serial_tenma_ps(args));
  if (name == "net")             return std::shared_ptr<Driver>(new Driver_net(args));
  throw Err() << "unknown driver: " << name;
}
