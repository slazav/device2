#ifndef DRV_NET_H
#define DRV_NET_H

#include <memory>
#include "drv.h"
#include "drv_utils.h"
#include "opt/opt.h"

class Driver_net: public Driver {
protected:
  int sockfd; // file descriptor for the network socket
  size_t bufsize;
  double timeout;
  std::string errpref,idn;
  std::string add,trim;
  read_cond_t read_cond;
  double delay;

public:

  Driver_net(const Opt & opts);
  ~Driver_net();

  std::string read();
  void write(const std::string & msg);
  std::string ask(const std::string & msg) override;
};

#endif
