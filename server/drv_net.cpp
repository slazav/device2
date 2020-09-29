#include "drv_net.h"

// read/write/open/close/fctl
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <termios.h>

// strerror
#include <cstring>

Driver_net::Driver_net(const Opt & opts): opts(opts) {
  net.reset(new IONet(opts));
}

Driver_net::~Driver_net() {
  net.reset();
}

std::string
Driver_net::ask(const std::string & msg) {
  if (!net) throw Err() << "device is closed";
  net->write(msg);

  // if we do not have '?' in the message
  // no answer is needed.
  if (msg.find('?') == std::string::npos)
    return std::string();

  return net->read();;
}
