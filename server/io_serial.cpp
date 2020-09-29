#include "io_serial.h"

// read/write/open/close/fctl
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <termios.h>

// strerror
#include <cstring>


IOSerial::IOSerial(const Opt & opts) {
  int ret;

  //prefix for error messages
  errpref = opts.get("errpref", "IOSerial: ");

  // open serial device
  std::string dev = opts.get("dev");
  if (dev == "") throw Err() << errpref
    << "Parameter -dev is empty or missing";

  errpref += dev + ": ";

  fd = ::open(dev.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
  if (fd<0) throw Err() << errpref
    << "can't open device: " << strerror(errno);

  // set non-blocking mode
  if (opts.exists("ndelay")){
    int fl = opts.get("ndelay", true) ? FNDELAY : 0;
    ret = fcntl(fd, F_SETFL, fl);
    if (ret<0) throw Err() << errpref
      << "can't do fcntl: " << strerror(errno);
  }

  //get current options
  struct termios options;
  ret = tcgetattr(fd, &options);
  if (ret<0) throw Err() << errpref
    << "can't get serial port parameters: " << strerror(errno);

  // set CLOCAL+CREAD flags
  options.c_cflag |= CLOCAL|CREAD;

  // set speed (should be done with cfset([io]?)speed)
  if (opts.exists("baud")){
    int pbaud = 0; // convert to posix mask
    switch (opts.get<int>("baud", 9600)) {
      case     0: pbaud =     B0; break;
      case    50: pbaud =    B50; break;
      case    75: pbaud =    B75; break;
      case   110: pbaud =   B110; break;
      case   134: pbaud =   B134; break;
      case   150: pbaud =   B150; break;
      case   200: pbaud =   B200; break;
      case   300: pbaud =   B300; break;
      case   600: pbaud =   B600; break;
      case  1200: pbaud =  B1200; break;
      case  1800: pbaud =  B1800; break;
      case  2400: pbaud =  B2400; break;
      case  4800: pbaud =  B4800; break;
      case  9600: pbaud =  B9600; break;
      case 19200: pbaud = B19200; break;
      case 38400: pbaud = B38400; break;
      // Extra output baud rates (not in POSIX)
      case 57600: pbaud = B57600; break;
      case 115200: pbaud = B115200; break;
      case 230400: pbaud = B230400; break;
      case 460800: pbaud = B460800; break;
      case 500000: pbaud = B500000; break;
      case 576000: pbaud = B576000; break;
      case 921600: pbaud = B921600; break;
      case 1000000: pbaud = B1000000; break;
      case 1152000: pbaud = B1152000; break;
      case 1500000: pbaud = B1500000; break;
      case 2000000: pbaud = B2000000; break;
      case 2500000: pbaud = B2500000; break;
      case 3000000: pbaud = B3000000; break;
      case 3500000: pbaud = B3500000; break;
      case 4000000: pbaud = B4000000; break;
      default: throw Err()
        << errpref << "unknown baud setting: "
        << opts.get("baud");
    }
    cfsetspeed(&options, pbaud);
  }

  // set parity
  if (opts.exists("parity")){
    std::string par  = opts.get("parity", "8N1");
    if (par == "8N1" || par == "7S1"){
      options.c_cflag &= ~PARENB;
      options.c_cflag &= ~CSTOPB;
      options.c_cflag &= ~CSIZE;
      options.c_cflag |= CS8;
    }
    else if (par == "7E1"){
      options.c_cflag |= PARENB;
      options.c_cflag &= ~PARODD;
      options.c_cflag &= ~CSTOPB;
      options.c_cflag &= ~CSIZE;
      options.c_cflag |= CS7;
    }
    else if (par == "7O1"){
      options.c_cflag |= PARENB;
      options.c_cflag |= PARODD;
      options.c_cflag &= ~CSTOPB;
      options.c_cflag &= ~CSIZE;
      options.c_cflag |= CS7;
    }
    else if (par != "") throw Err() << errpref
     << "unknown parity setting "
     << "(expected 8N1, 7E1, 7O1, or 7S1): " << par;
  }

  // enable parity checking and stripping of the parity bit
  if (options.c_cflag & PARENB)
    options.c_iflag |= (INPCK | ISTRIP);

  // canonical/raw input
  if (opts.exists("raw")){
    if (opts.get("raw", true))
      options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    else
      options.c_lflag |= (ICANON | ECHO | ECHOE);
  }

  // software flow control
  if (opts.exists("sfc")){
    if (opts.get("sfc", true))
      options.c_iflag |= (IXON | IXOFF | IXANY);
    else
      options.c_iflag &= ~(IXON | IXOFF | IXANY);
  }

  // raw output
  // todo: toupper, cr-nl conversions
  options.c_oflag &= ~OPOST;

  // set timeout
  if (opts.exists("timeout")){
    double timeout = opts.get<double>("timeout", 5.0);
    if (timeout < 0 || timeout > 25.5)
      throw Err() << errpref << "timeout 0..25.5 s expected";
    options.c_cc[VMIN]  = 0;
    options.c_cc[VTIME] = timeout*10; // convert to 1/10 of seconds
  }

  // set parameters
  ret=tcsetattr(fd, TCSANOW, &options);
  if (ret<0) throw Err() << errpref
    << "can't set serial port parameters: " << strerror(errno);

  // set bufsize
  bufsize = opts.get("bufsize", 4096);
}


IOSerial::~IOSerial() {
  ::close(fd);
}


std::string
IOSerial::read() {
  char buf[bufsize];
  ssize_t ret = ::read(fd,buf,sizeof(buf));
  if (ret<0) throw Err() << errpref
    << "read error: " << strerror(errno);
  return std::string(buf, buf+ret);
}

void
IOSerial::write(const std::string & msg) {
  ssize_t ret = ::write(fd, msg.data(), msg.size());
  if (ret<0) throw Err() << errpref
    << "write error: " << strerror(errno);
}
