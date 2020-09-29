#include "drv_tenma_ps.h"
#include <algorithm>

// read/write/open/close/fctl
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// strerror
#include <cstring>


Driver_tenma_ps::Driver_tenma_ps(const Opt & opts) {
  //prefix for error messages
  errpref = opts.get("errpref", "usbtmc: ");

  auto dev = opts.get("dev");
  if (dev == "") throw Err() << errpref
    << "parameter -dev is empty or missing";

  errpref += dev + ": ";

  del=250000; // 0.25s

  fd = ::open(dev.c_str(), O_RDWR);
  if (fd<0) throw Err() << errpref
    << "can't open device: " << strerror(errno);

  // set non-blocking mode
  int ret = fcntl(fd, F_SETFL, FNDELAY);
  if (ret<0) throw Err() << errpref
    << "can't do fcntl: " << strerror(errno);
}


Driver_tenma_ps::~Driver_tenma_ps() {
  ::close(fd);
}

std::string
Driver_tenma_ps::read() {
  char buf[4096];
  auto ret = ::read(fd,buf,sizeof(buf));
  if (ret<0) throw Err() << errpref
    << "read error: " << strerror(errno);

  return std::string(buf, buf+ret);
}

void
Driver_tenma_ps::write(const std::string & msg){
  // to upper case
  std::string m(msg);
  std::transform(m.begin(),m.end(),m.begin(),::toupper);

  auto ret = ::write(fd, m.data(), m.size());
  if (ret<0) throw Err() << errpref
    << "write error: " << strerror(errno);

  // delay is needed after writing
  usleep(del);
}


std::string
Driver_tenma_ps::ask(const std::string & msg) {
  write(msg); // no newline!

  // if we do not have '?' in the message
  // no answer is needed.
  if (msg.find('?') == std::string::npos)
    return std::string();

  auto ret = read();

  // convert binary status byte to a number
  if (msg == "STATUS?" && ret.size()>0) {
    return type_to_str((int)ret[0]);
  }

  return ret;
}
