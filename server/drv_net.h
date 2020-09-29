#ifndef DRV_NET_H
#define DRV_NET_H

#include <memory>
#include "drv.h"
#include "opt/opt.h"
#include "io_net.h"

/*************************************************/
// Network driver

class Driver_net: public Driver {
  Opt opts;
  std::shared_ptr<IONet> net;

public:

  Driver_net(const Opt & opts);
  void open() override;
  void close() override;
  std::string ask(const std::string & msg) override;
};

#endif
