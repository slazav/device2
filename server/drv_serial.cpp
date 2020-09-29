#include "drv_serial.h"

// read/write/open/close/fctl
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <termios.h>

// strerror
#include <cstring>

Driver_serial::Driver_serial(const Opt & opts): opts(opts) {}

void
Driver_serial::open() { serial.reset(new IOSerial(opts)); }

void
Driver_serial::close() { serial.reset(); }

std::string
Driver_serial::ask(const std::string & msg) {
  if (!serial) throw Err() << "device is closed";
  serial->write(msg + "\n");

  // if we do not have '?' in the message
  // no answer is needed.
  if (msg.find('?') == std::string::npos)
    return std::string();

  return serial->read();;
}
