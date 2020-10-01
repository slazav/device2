#include "drv_serial.h"
#include "drv_utils.h"

// read/write/open/close/fctl
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <termios.h>

// strerror
#include <cstring>



// If option "oname" exists, use its bool value
// to set par bits according to the mask.
void
set_opt(const Opt & opts,
        const std::string & oname,
        unsigned int * par, const unsigned int mask){

  if (opts.exists(oname)){
    if (opts.get(oname, true)) *par |= mask;
    else *par &= ~mask;
  }
}

// convert integer baud rate to mask.
// return -1 on error
int
baud_to_mask(int b){
  switch (b) {
    case     0: return B0;
    case    50: return B50;
    case    75: return B75;
    case   110: return B110;
    case   134: return B134;
    case   150: return B150;
    case   200: return B200;
    case   300: return B300;
    case   600: return B600;
    case  1200: return B1200;
    case  1800: return B1800;
    case  2400: return B2400;
    case  4800: return B4800;
    case  9600: return B9600;
    case 19200: return B19200;
    case 38400: return B38400;
    // Extra output baud rates (not in POSIX)
    case 57600: return B57600;
    case 115200: return B115200;
    case 230400: return B230400;
    case 460800: return B460800;
    case 500000: return B500000;
    case 576000: return B576000;
    case 921600: return B921600;
    case 1000000: return B1000000;
    case 1152000: return B1152000;
    case 1500000: return B1500000;
    case 2000000: return B2000000;
    case 2500000: return B2500000;
    case 3000000: return B3000000;
    case 3500000: return B3500000;
    case 4000000: return B4000000;
  }
  return -1;
}

Driver_serial::Driver_serial(const Opt & opts) {
  opts.check_unknown({"errpref","idn","dev","ndelay",
    "speed","ispeed","ospeed", // speed
    "clocal","cread","crtscts","cs","cstopb","hup", "parenb","parodd","cmspar", //control
    "icrnl","inlcr","igncr","iuclc","iutf8","brkint","ignbrk","imaxbel",
    "inpck","ignpar","istrip","parmrk","ixany","ixoff","ixon", // input
    "bs","cr","ff","nl","tab","vt","ocrnl","onlcr","onlret","onocr",
    "ofdel","ofill","olcuc","opost", // output
    "echo","echoctl","echoe","echok","echoke","echonl","echoprt","extproc",
    "flusho","icanon","iexten","isig","noflsh","tostop","xcase", // local
    "parity","raw","sfc","nlcnv","lcase","timeout","vmin",
    "delay","add_str","trim_str","ack_str","nack_str"});
  int ret;

  //prefix for error messages
  errpref = opts.get("errpref", "serial: ");

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

  // baud rate

  // set speed
  if (opts.exists("speed")){
    int pbaud = baud_to_mask(opts.get<int>("speed", -1));
    if (pbaud==-1) throw Err()
        << errpref << "unknown baud setting: "
        << opts.get("baud");
    cfsetspeed(&options, pbaud);
  }

  // set input speed
  if (opts.exists("ispeed")){
    int pbaud = baud_to_mask(opts.get<int>("ispeed", -1));
    if (pbaud==-1) throw Err()
        << errpref << "unknown baud setting: "
        << opts.get("baud");
    cfsetispeed(&options, pbaud);
  }

  // set output speed
  if (opts.exists("ospeed")){
    int pbaud = baud_to_mask(opts.get<int>("ospeed", -1));
    if (pbaud==-1) throw Err()
        << errpref << "unknown baud setting: "
        << opts.get("baud");
    cfsetospeed(&options, pbaud);
  }

  // Control settings

  set_opt(opts, "clocal",  &options.c_cflag, CLOCAL);
  set_opt(opts, "cread",   &options.c_cflag, CREAD);
  set_opt(opts, "crtscts", &options.c_cflag, CRTSCTS);

  if (opts.exists("cs")){
    int v=0;
    switch (opts.get("cs", 0)){
      case 5:  v=CS5; break;
      case 6:  v=CS6; break;
      case 7:  v=CS7; break;
      case 8:  v=CS8; break;
      default: throw Err() << errpref
        << "character size should be in [5..8]: " << opts.get("cs");
    }
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= v;
  }
  set_opt(opts, "cstopb", &options.c_cflag, CSTOPB);
  set_opt(opts, "hup",    &options.c_cflag, HUPCL);
  set_opt(opts, "parenb", &options.c_cflag, PARENB);
  set_opt(opts, "parodd", &options.c_cflag, PARODD);
  set_opt(opts, "cmspar", &options.c_cflag, CMSPAR);

  // Input settings

  set_opt(opts, "icrnl",  &options.c_iflag, ICRNL);
  set_opt(opts, "inlcr",  &options.c_iflag, INLCR);
  set_opt(opts, "igncr",  &options.c_iflag, IGNCR);
  set_opt(opts, "iuclc",  &options.c_iflag, IUCLC);
  set_opt(opts, "iutf8",  &options.c_iflag, IUTF8);
  set_opt(opts, "brkint", &options.c_iflag, BRKINT);
  set_opt(opts, "ignbrk", &options.c_iflag, IGNBRK);
  set_opt(opts, "imaxbel",&options.c_iflag, IMAXBEL);
  set_opt(opts, "inpck",  &options.c_iflag, INPCK);
  set_opt(opts, "ignpar", &options.c_iflag, IGNPAR);
  set_opt(opts, "istrip", &options.c_iflag, ISTRIP);
  set_opt(opts, "parmrk", &options.c_iflag, PARMRK);
  set_opt(opts, "ixany",  &options.c_iflag, IXANY);
  set_opt(opts, "ixoff",  &options.c_iflag, IXOFF);
  set_opt(opts, "ixon",   &options.c_iflag, IXON);


  // Output settings

  if (opts.exists("bs")){
    int v = 0;
    switch (opts.get("bs", -1)){
      case 0: v = BS0; break;
      case 1: v = BS1; break;
      default: throw Err() << errpref
        << "backspace delay style should be in [0..1]: " << opts.get("bs");
    }
    options.c_oflag &= ~BSDLY;
    options.c_oflag |= v;
  }

  if (opts.exists("cr")){
    int v = 0;
    switch (opts.get("cr", -1)){
      case 0: v = CR0; break;
      case 1: v = CR1; break;
      case 2: v = CR2; break;
      case 3: v = CR3; break;
      default: throw Err() << errpref
        << "carriage return delay style should be in [0..3]: " << opts.get("cr");
    }
    options.c_oflag &= ~CRDLY;
    options.c_oflag |= v;
  }

  if (opts.exists("ff")){
    int v = 0;
    switch (opts.get("ff", -1)){
      case 0: v = FF0; break;
      case 1: v = FF1; break;
      default: throw Err() << errpref
        << "form feed style should be in [0..1]: " << opts.get("ff");
    }
    options.c_oflag &= ~FFDLY;
    options.c_oflag |= v;
  }

  if (opts.exists("nl")){
    int v = 0;
    switch (opts.get("nl", -1)){
      case 0:  v = NL0; break;
      case 1:  v = NL1; break;
      default: throw Err() << errpref
        << "newline delay style should be in [0..1]: " << opts.get("nl");
    }
    options.c_oflag &= ~NLDLY;
    options.c_oflag |= v;
  }

  if (opts.exists("tab")){
    int v = 0;
    switch (opts.get("tab", -1)){
      case 0: v = TAB0; break;
      case 1: v = TAB1; break;
      case 2: v = TAB2; break;
      case 3: v = TAB3; break;
      default: throw Err() << errpref
        << "tab delay style should be in [0..3]: " << opts.get("tab");
    }
    options.c_oflag &= ~TABDLY;
    options.c_oflag |= v;
  }

  if (opts.exists("vt")){
    int v = 0;
    switch (opts.get("vt", -1)){
      case 0:  v = VT0; break;
      case 1:  v = VT1; break;
      default: throw Err() << errpref
        << "vertical tab delay style should be in [0..1]: " << opts.get("vt");
    }
    options.c_oflag &= ~VTDLY;
    options.c_oflag |= v;
  }

  set_opt(opts, "ocrnl",  &options.c_oflag, OCRNL);
  set_opt(opts, "onlcr",  &options.c_oflag, ONLCR);
  set_opt(opts, "onlret", &options.c_oflag, ONLRET);
  set_opt(opts, "onocr",  &options.c_oflag, ONOCR);
  set_opt(opts, "ofdel",  &options.c_oflag, OFDEL);
  set_opt(opts, "ofill",  &options.c_oflag, OFILL);
  set_opt(opts, "olcuc",  &options.c_oflag, OLCUC);
  set_opt(opts, "opost",  &options.c_oflag, OPOST);

  // Local settings

  set_opt(opts, "echo",    &options.c_lflag, ECHO);
  set_opt(opts, "echoctl", &options.c_lflag, ECHOCTL);
  set_opt(opts, "echoe",   &options.c_lflag, ECHOE);
  set_opt(opts, "echok",   &options.c_lflag, ECHOK);
  set_opt(opts, "echoke",  &options.c_lflag, ECHOKE);
  set_opt(opts, "echonl",  &options.c_lflag, ECHONL);
  set_opt(opts, "echoprt", &options.c_lflag, ECHOPRT);
  set_opt(opts, "extproc", &options.c_lflag, EXTPROC);
  set_opt(opts, "flusho",  &options.c_lflag, FLUSHO);
  set_opt(opts, "icanon",  &options.c_lflag, ICANON);
  set_opt(opts, "iexten",  &options.c_lflag, IEXTEN);
  set_opt(opts, "isig",    &options.c_lflag, ISIG);
  set_opt(opts, "noflsh",  &options.c_lflag, NOFLSH);
  set_opt(opts, "tostop",  &options.c_lflag, TOSTOP);
  set_opt(opts, "xcase",   &options.c_lflag, XCASE);

  // combination settings

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
  set_opt(opts, "sfc", &options.c_iflag, IXON | IXOFF | IXANY);

  // nl conversion
  set_opt(opts, "nlcnv", &options.c_iflag, ICRNL);
  set_opt(opts, "nlcnv", &options.c_oflag, ONLCR);

  // case conversion
  set_opt(opts, "lcase", &options.c_iflag, IUCLC);
  set_opt(opts, "lcase", &options.c_oflag, OLCUC);

  // set timeout
  if (opts.exists("timeout")){
    double timeout = opts.get<double>("timeout", 5.0);
    if (timeout < 0 || timeout > 25.5)
      throw Err() << errpref << "timeout 0..25.5 s expected";
    options.c_cc[VTIME] = timeout*10; // convert to 1/10 of seconds
  }

  // set vmin
  if (opts.exists("vmin")){
    int v = opts.get<>("vmin", 0);
    if (v < 0 || v > 255)
      throw Err() << errpref << "vmin should be in [0..255]";
    options.c_cc[VMIN]  = v;
  }

  // write parameters
  ret=tcsetattr(fd, TCSANOW, &options);
  if (ret<0) throw Err() << errpref
    << "can't set serial port parameters: " << strerror(errno);

  delay  = opts.get("delay",  0.1);
  add    = opts.get("add_str");
  trim   = opts.get("trim_str");
  ack    = opts.get("ack_str");
  nack   = opts.get("nack_str");
  idn    = opts.get("idn", "");
  read_cond = str_to_read_cond(opts.get("read_cond", "always"));
}


Driver_serial::~Driver_serial() {
  ::close(fd);
}


std::string
Driver_serial::read() {

  std::string ret;
  bool fail = false;

  while(1){
    // read data, add to ret string
    char buf[4096]; // limit of the serial driver
    ssize_t res = ::read(fd,buf,sizeof(buf));
    if (res<0) throw Err() << errpref
      << "read error: " << strerror(errno);
    if (res==0) throw Err() << errpref
      << "read timeout";
    ret += std::string(buf, buf+res);

    // stop reading if -ack option is not set
    if (ack.size()==0) break;

    // if data ends with ack
    if (trim_str(ret, ack)) {break;}

    // if data ends with nack
    if (trim_str(ret,nack)) {fail=true; break;}

    // read more data if nack or ack are not found.
  }

  trim_str(ret,trim); // -trim option
  if (fail) throw Err() << "nack from the device: " << ret;
  return ret;
}

void
Driver_serial::write(const std::string & msg) {

  std::string m = msg;
  if (add.size() > 0) m+=add;

  ssize_t ret = ::write(fd, m.data(), m.size());
  if (ret<0) throw Err() << errpref
    << "write error: " << strerror(errno);

  usleep(delay*1e6);
}

std::string
Driver_serial::ask(const std::string & msg) {

  if (idn.size() && strcasecmp(msg.c_str(),"*idn?")==0) return idn;

  write(msg);

  if (!check_read_cond(msg, read_cond)) return std::string();

  return read();
}
