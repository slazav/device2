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

  // Try to turn on auto_abort feature of usbtmc driver to
  // automatically recover after failed i/o operations.
  // The feature appeares in 9.2018 and can be missing in
  // old kernels.
  uint8_t c = 1;
  ret = ioctl(fd, USBTMC_IOCTL_AUTO_ABORT, &c);
  auto_abort = (ret==0);

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
      auto en = errno; // save errno to show the error later
      // Recover from failed read (if auto_abort is off).
      if (!auto_abort) ioctl(fd,USBTMC_IOCTL_CLEAR,NULL);
      throw Err() << errpref
        << "read error: " << strerror(en);
    }
    ret += std::string(buf, buf+res);

    // Sometimes STB is set after a short delay after read
    // Wait for 10 ms
    usleep(10000);

    // Check if more data is available (bit4 of STB)
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
  if (res<0){
    auto en = errno; // save errno to show the error later
    // Recover from failed read (if auto_abort is off).
    if (!auto_abort) ioctl(fd,USBTMC_IOCTL_CLEAR,NULL);
    throw Err() << errpref
      << "read error: " << strerror(en);
  }
}

std::string
Driver_usbtmc::ask(const std::string & msg) {

  if (idn.size() && strcasecmp(msg.c_str(),"*idn?")==0) return idn;

  write(msg);

  // if there is no '?' in the message no answer is needed.
  if (!check_read_cond(msg, READCOND_QMARK1W)) return std::string();

  return read();
}


