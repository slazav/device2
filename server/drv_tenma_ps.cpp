#include "drv_tenma_ps.h"
#include <algorithm>

// read/write/open/close/fctl
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// strerror
#include <cstring>


Driver_tenma_ps::Driver_tenma_ps(const Opt & opts): Driver(opts) {
  dev = opts.get("dev");
  if (dev == "") throw Err() << "Parameter -dev is empty or missing";
  del=250000; // 0.25s
}


void
Driver_tenma_ps::open() {
  fd = ::open(dev.c_str(), O_RDWR);
  if (fd<0) throw Err() << "tenma_ps driver: can't open device: "
                        << dev << ": " << strerror(errno);
  // set non-blocking mode
  int ret = fcntl(fd, F_SETFL, FNDELAY);
  if (ret<0) throw Err() << "tenma_ps driver: can't do fcntl: "
                         << dev << ": " << strerror(errno);
}


void
Driver_tenma_ps::close() {
  ::close(fd);
}


std::string
Driver_tenma_ps::ask(const std::string & msg) {

  // to upper case
  std::string m(msg);
  std::transform(m.begin(),m.end(),m.begin(),::toupper);

  ssize_t ret = write(fd, m.data(), m.size());
  if (ret<0) throw Err() << "tenma_ps driver: write error: "
                         << dev << ": " << strerror(errno);

  // delay is needed after writing
  usleep(del);

  // if we do not have '?' in the message
  // no answer is needed.
  if (msg.find('?') == std::string::npos)
    return std::string();

  char buf[4096];
  ret = read(fd,buf,sizeof(buf));
  if (ret<0) throw Err() << "tenma_ps driver: read error: "
                         << dev << ": " << strerror(errno);

  // convert binary status byte to a number
  if (msg == "STATUS?" && ret>0) {
    return type_to_str((int)buf[0]);
  }
  return std::string(buf, buf+ret);
}
