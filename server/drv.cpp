#include "drv.h"
#include "drv_test.h"
#include "drv_spp.h"
#include "drv_usbtmc.h"
#include "drv_net.h"
#include "drv_net_gpib_prologix.h"
#include "drv_serial.h"
#include "drv_serial_asm340.h"
#include "drv_serial_l300.h"
#include "drv_serial_vs_ld.h"
#include "drv_serial_tenma_ps.h"
#include "drv_serial_et.h"
#include "drv_serial_simple.h"
#include "drv_gpib.h"

std::shared_ptr<Driver>
Driver::create(const std::string & name, const Opt & args){

  if (name == "test")
    return std::shared_ptr<Driver>(new Driver_test(args));

  if (name == "spp")
    return std::shared_ptr<Driver>(new Driver_spp(args));

  if (name == "usbtmc")
    return std::shared_ptr<Driver>(new Driver_usbtmc(args));

  if (name == "serial")
    return std::shared_ptr<Driver>(new Driver_serial(args));

  if (name == "serial_asm340")
    return std::shared_ptr<Driver>(new Driver_serial_asm340(args));

  if (name == "serial_l300")
    return std::shared_ptr<Driver>(new Driver_serial_l300(args));

  if (name == "serial_vs_ld")
    return std::shared_ptr<Driver>(new Driver_serial_vs_ld(args));

  if (name == "serial_tenma_ps")
    return std::shared_ptr<Driver>(new Driver_serial_tenma_ps(args));

  if (name == "serial_et")
    return std::shared_ptr<Driver>(new Driver_serial_et(args));

  if (name == "serial_simple")
    return std::shared_ptr<Driver>(new Driver_serial_simple(args));

  if (name == "net")
     return std::shared_ptr<Driver>(new Driver_net(args));

  if (name == "net_gpib_prologix")
     return std::shared_ptr<Driver>(new Driver_net_gpib_prologix(args));

#ifdef USE_GPIB
  if (name == "gpib")
     return std::shared_ptr<Driver>(new Driver_gpib(args));
#endif

  throw Err() << "unknown driver: " << name;
}
