#ifndef DRV_SERIAL_H
#define DRV_SERIAL_H

#include "drv.h"

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

class IONet {
  int sockfd; // file descriptor for the network socket
  std::string errpref;
  size_t bufsize;
  double timeout;

public:

  IONet(const Opt & opts);
  ~IONet();
  std::string read();
  void write(const std::string & msg);
};

#endif
