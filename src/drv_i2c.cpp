#include "drv_i2c.h"
#include "drv_utils.h"

#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h>
#include <errno.h>
#include <cstring>
#include <iomanip>

/************************************************/

std::map<dev_t, std::unique_ptr<std::mutex> >
Driver_i2c::mtxs;


Driver_i2c::Driver_i2c(const Opt & opts) {

  opts.check_unknown({"dev", "addr", "errpref", "idn", "hex"});

  dev = opts.get("dev");
  if (dev == "") throw Err() << errpref
    << "parameter -dev is empty or missing";

  addr = opts.get<unsigned char>("addr", 255);
  if (addr > 127) throw Err() << errpref
    << "parameter -addr 0..127 should be set";

  // open i2c device
  fd = ::open(dev.c_str(), O_RDWR);
  if (fd < 0) throw Err()
    << "can't open i2c device: " << dev << ": " << strerror(errno);

  // save st_dev of the file (major+minor), to lock access properly
  struct stat st;
  if (fstat(fd, &st)!=0) throw Err()
    << "can't do stat for i2c device: " << dev << ": " << strerror(errno);
  st_dev = st.st_dev;

  //set other parameters
  errpref = opts.get("errpref", "i2c: ");
  idn = opts.get("idn", "");
  hex = opts.get<bool>("hex", 0);
}

Driver_i2c::~Driver_i2c() {
  ::close(fd);
}

/************************************************/

// I'm using plain i2c communication via read/write interface. Address is set
// previously with I2C_SLAVE ioctl call. Another option would be to use I2C_RDWR call.
// One more possibility - to use SMBus interface via ioctl of i2c library.

void
Driver_i2c::i2c_set_addr(unsigned char addr){
   if (addr>127) throw Err()
      << "i2c address out of range (0..127): " << addr;

  if (ioctl(fd, I2C_SLAVE, addr) < 0) throw Err()
    << "can't use i2c address: " << (int)addr << ": " << strerror(errno);
}

void
Driver_i2c::i2c_write(const unsigned char data[], const size_t size){
  if (::write(fd, data, size) == -1)
    throw Err() << "error in i2c write: " << strerror(errno);
}

std::string
Driver_i2c::i2c_read(size_t count){
  std::string rbuf(count, '\0');
  auto res = ::read(fd, (void *)rbuf.data(), rbuf.size());
  if (res == -1) throw Err() << "error in i2c read: " << strerror(errno);
  return rbuf;
}

/************************************************/

std::string
Driver_i2c::ask(const std::string & msg) {

  // *idn? command
  if (idn.size() && strcasecmp(msg.c_str(),"*idn?")==0) return idn;

  // Split command into words:
  std::vector<std::string> cmd;
  std::istringstream ss(msg);
  while (1){
    ss >> std::ws;
    if (ss.eof()) break;

    std::string v;
    ss >> v >> std::ws;
    if (ss.bad()) throw Err()
      << "can't parse command: " << msg;
    cmd.push_back(v);
    if (ss.eof()) break;
  }

  // get lock
  auto lock = get_lock();

  // set address
  i2c_set_addr(addr);

  std::string ret;
  size_t start = 0;
  while (1){
    if (start+1 > cmd.size()) break;

    if (cmd[start] == "read"){
      if (start+2 > cmd.size())
        throw Err() << "argument for read command missing: " << msg;

      auto count = str_to_type<size_t>(cmd[start+1]);
      auto rbuf = i2c_read(count);

      // convert rbuf into bytes
      for (size_t i = 0; i < count; i++)
        ret += (ret.size()==0? "":" ")
          + (hex ? type_to_str_hex<uint8_t>(rbuf[i]): type_to_str<uint8_t>(rbuf[i]));

      start+=2;
      continue;
    }

    if (cmd[start] == "write"){
      std::string wbuf;
      size_t i;
      for (i = start+1; i<cmd.size() && cmd[i]!="read" && cmd[i]!="write"; i++)
        wbuf += str_to_type<uint8_t>(cmd[i]);
      i2c_write(wbuf);
      start = i;
      continue;
    }

    throw Err() << "read or write command expected: " << msg;
  }

  return ret;
};

