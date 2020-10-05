#include "drv_usbtmc.h"
#include "drv_utils.h"
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
  opts.check_unknown({"dev", "timeout", "errpref", "idn", "add_str", "trim_str"});

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

  // set timeout
  uint32_t v = opts.get<double>("timeout", 5.0)*1000; // convert to integer ms
  ret = ioctl(fd, USBTMC_IOCTL_SET_TIMEOUT, &v);
  if (ret<0) throw Err() << errpref
    << "can't set timeout: " << strerror(errno);

// This should be correct way to recover from read timeouts;
// But it does not work fo me (Invalid request code)
//
//  // Abort I/O operations on timeouts:
//  uint8_t autoabrt = 1;
//  ret = ioctl(fd, USBTMC_IOCTL_AUTO_ABORT, &autoabrt);
//  if (ret<0) throw Err() << errpref
//    << "can't set IOCTL_AUTO_ABORT: " << strerror(errno);

  add     = opts.get("add_str",  "\n");
  trim    = opts.get("trim_str", "\n");
}


Driver_usbtmc::~Driver_usbtmc() {
  ::close(fd);
}

std::string
Driver_usbtmc::read() {
  char buf[4096];

  std::string ret;
  while (1) {
    auto res = ::read(fd,buf,sizeof(buf));
    if (res<0){
      auto en = errno;
      // Recover from timeout.
      // Documentation says that USBTMC_IOCTL_ABORT_BULK_IN
      // should be enough, but for me it does not work in some cases.
      ioctl(fd,USBTMC_IOCTL_CLEAR,NULL);
      throw Err() << errpref
        << "read error: " << strerror(en);
    }
    ret += std::string(buf, buf+res);

    // Check if more data is available (bit4 - data available)
    // This loop is needed for some slow operations
    // (such as Keysight multiplexer read? command)
    uint8_t stb;
    res = ioctl(fd,USBTMC488_IOCTL_READ_STB, &stb);
    if (res<0) throw Err() << errpref
      << "can't get status byte: " << strerror(errno);
    if (!(stb & (1<<4))) break;
  }

  trim_str(ret,trim); // -trim option

  return ret;
}

void
Driver_usbtmc::write(const std::string & msg) {

  std::string m = msg;
  if (add.size()>0) m+=add;

  auto res = ::write(fd, m.data(), m.size());
  if (res<0) throw Err() << errpref
    << "write error: " << strerror(errno);
}

std::string
Driver_usbtmc::ask(const std::string & msg) {

  if (idn.size() && strcasecmp(msg.c_str(),"*idn?")==0) return idn;

  write(msg);

  // if there is no '?' in the message no answer is needed.
  if (!check_read_cond(msg, READCOND_QMARK1W)) return std::string();

  return read();
}


