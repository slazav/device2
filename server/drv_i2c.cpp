#include "drv_i2c.h"
#include "drv_utils.h"

#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>

/* command examples:
bmp280: read calibration parameters (24 bytes)
  device_c ask i2c0 0x3C 0x88 12 -- display
  device_c ask i2c0 0x76 0x88 12 -- bmp280
*/

Driver_i2c::Driver_i2c(const Opt & opts) {

  opts.check_unknown({"dev", "errpref", "idn", "hex"});

  // open i2c device
  auto dev = opts.get("dev");
  if (dev == "") throw Err() << errpref
    << "parameter -dev is empty or missing";

  fd = ::open(dev.c_str(), O_RDWR);
  if (fd < 0) throw Err()
    << "can't open i2c device: " << dev << ": " << strerror(errno);

  //set other parameters
  errpref = opts.get("errpref", "i2c: ");
  idn = opts.get("idn", "");
  hex = opts.get<bool>("hex", 0);
}

Driver_i2c::~Driver_i2c() {
  ::close(fd);
}

std::string
Driver_i2c::ask(const std::string & msg) {

  // *idn? command
  if (idn.size() && strcasecmp(msg.c_str(),"*idn?")==0) return idn;

  // Split command into words:
  //   <addr> [<bytes to write> ...] <read count>
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

  // Check number of words, calculate number of byted to write:
  if (cmd.size()<2) throw Err()
      << "not enough arguments: " << msg;
  size_t wlen = cmd.size()-2;

  // last argument - number of bytes to read:
  auto rlen = str_to_type<unsigned int>(cmd[cmd.size()-1]);

  // I2C address
  auto addr = str_to_type<unsigned char>(cmd[0]);
  if (addr>127) throw Err()
      << "i2c address out of range (0..127): " << cmd[0];

  if (ioctl(fd, I2C_SLAVE, addr) < 0) throw Err()
    << "can't use i2c address: " << (int)addr << ": " << strerror(errno);

  // I'm using plain i2c communication via read/write interface. Address is set
  // previously with I2C_SLAVE ioctl call. Another option would be to use I2C_RDWR call.
  // One more possibility - to use SMBus interface via ioctl of i2c library.

  std::vector<struct i2c_msg> mbuf; // buffer to keep segments (0, 1, or 2)
  std::string wbuf(wlen, '\0'), rbuf(rlen, '\0'); // buffers to keep data

  // fill wbuf
  for (size_t i = 0; i < wlen; i++) wbuf[i] = str_to_type<uint8_t>(cmd[i+1]);

  // Write data if needed
  if (wlen>0){
    auto res = ::write(fd, wbuf.data(), wbuf.size());
    if (res == -1) throw Err() << "error in i2c write: " << strerror(errno);
  }

  // Read data if needed
  if (rlen>0){
    auto res = ::read(fd, (void *)rbuf.data(), rbuf.size());
    if (res == -1) throw Err() << "error in i2c read: " << strerror(errno);
  }

  // convert rbuf into bytes
  std::string ret;
  for (size_t i = 0; i < rlen; i++)
    ret += (i==0? "":" ")
        + (hex ? type_to_str_hex<uint8_t>(rbuf[i]): type_to_str<uint8_t>(rbuf[i]));

  return ret;

};
