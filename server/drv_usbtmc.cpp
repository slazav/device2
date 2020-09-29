#include "drv_usbtmc.h"
#include "tmc.h" // from linux kernel


// read/write/open/close/fctl
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/ioctl.h>

// strerror
#include <cstring>

Driver_usbtmc::Driver_usbtmc(const Opt & opts) {

  //prefix for error messages
  errpref = opts.get("errpref", "usbtmc: ");

  auto dev = opts.get("dev");
  if (dev == "") throw Err() << errpref
    << "parameter -dev is empty or missing";

  errpref += dev + ": ";

  fd = ::open(dev.c_str(), O_RDWR);
  if (fd<0) throw Err() << errpref
    << "can't open device: " << strerror(errno);

  // clear input and output buffers
  ssize_t ret = ioctl(fd, USBTMC_IOCTL_CLEAR, NULL);
  if (ret<0) throw Err() << errpref
    << "can't clear device state: " << strerror(errno);

  // set timeout (ms)
  if (opts.exists("timeout")) {
    uint32_t v = opts.get<double>("read_timeout", 0)*1000; // convert to integer ms
    ret = ioctl(fd, USBTMC_IOCTL_SET_TIMEOUT, &v);
    if (ret<0) throw Err() << errpref
      << "can't set timeout: " << strerror(errno);
  }
}


Driver_usbtmc::~Driver_usbtmc() {
  ::close(fd);
}


std::string
Driver_usbtmc::ask(const std::string & msg) {
  if (msg.size()==0) return "";
  // add '\n' if needed
  std::string m = msg;
  if (msg[msg.size()-1]!='\n') m+='\n';
  ssize_t ret = write(fd, m.data(), m.size());
  if (ret<0) throw Err() << errpref
    << "write error: " << strerror(errno);

  // if we do not have '?' in the message
  // no answer is needed.
  if (msg.find('?') == std::string::npos)
    return std::string();

  char buf[4096];
  ret = read(fd,buf,sizeof(buf));
  if (ret<0){
    auto en = errno;
    // Recover from timeout.
    // Documentation says that USBTMC_IOCTL_ABORT_BULK_IN
    // should be enough, but for me it does not work in some cases.
    ioctl(fd,USBTMC_IOCTL_CLEAR,NULL);
    throw Err() << errpref
      << "read error: " << strerror(en);
  }
  // remove '\n' is needed
  if (buf[ret-1]=='\n') ret--;
  return std::string(buf, buf+ret);
}
