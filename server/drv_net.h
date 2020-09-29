#ifndef DRV_NET_H
#define DRV_NET_H

#include <memory>
#include "drv.h"
#include "opt/opt.h"

/*************************************************/
/*
 * Access to a network device
 *
 * see https://beej.us/guide/bgnet/html//index.html

Options:
  -addr    -- Network address or IP.
              Required.
  -port    -- Port number.
              Required.
  -timeout -- Read timeout, seconds. No timeout if <=0.
              Default 5.0.
  -bufsize -- Buffer size for reading. Maximum length of read data.
              Default: 4096
  -errpref -- Prefix for error messages.
              Default: "IOSerial: "
*/

class Driver_net: public Driver {
  int sockfd; // file descriptor for the network socket
  size_t bufsize;
  double timeout;
  std::string errpref;

public:

  Driver_net(const Opt & opts);
  ~Driver_net();

  std::string read();
  void write(const std::string & msg);

  std::string ask(const std::string & msg) override;
};

#endif
