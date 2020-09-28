#ifndef DRIVERS_H
#define DRIVERS_H

#include <string>
#include <map>
#include <memory>
#include <cstring>

#include "log/log.h"
#include "opt/opt.h"
#include "iofilter/iofilter.h"

/*************************************************/
// base class

class Driver {
protected:
  Opt opts;
  Driver(const Opt & opts): opts(opts) {}

public:
  // Create and return a device driver (static function).
  // Parameter `args` contains parameters from the configuration file.
  // They describe how the driver can find the device.
  static std::shared_ptr<Driver> create(
    const std::string & name, const Opt & args);

  // Open device.
  virtual void open() = 0;

  // Close device.
  virtual void close() = 0;

  // Send message to the device, get answer
  virtual std::string ask(const std::string & msg) = 0;

};

/*************************************************/
// Test driver
// Configuration options: none
// Actions:
//   ask - just return the action argument

struct Driver_test: Driver {
private:
  bool opened;
public:
  Driver_test(const Opt & opts): Driver(opts) {}
  void open()  override {opened=true;}
  void close() override {opened=false;}
  std::string ask(const std::string & msg) override {
    if (!opened) throw Err() << "Test: device if closed";
    return msg;
  };
};

/*************************************************/
// SPP driver
// This driver implements "Simple pipe prococol" for comminicating with
// programs using stdin/stdout unix pipes.
// Used in a few of my projects (pico_rec, graphene),
// described in https://github.com/slazav/tcl-device (see Readme.md)
// Options:
//  -prog          -- program name
//  -open_timeout  -- timeout for opening
//  -read_timeout  -- timeout for reading

struct Driver_spp: Driver {
  std::shared_ptr<IOFilter> flt;
  std::string prog;
  char ch; // protocol special character
  int ver; // protocol version
  double open_timeout, read_timeout;

  Driver_spp(const Opt & opts): Driver(opts) {
    prog = opts.get("prog");
    open_timeout = opts.get<double>("open_timeout", 20.0);
    read_timeout = opts.get<double>("read_timeout", 5.0);
    if (prog == "") throw Err() << "Parameter -prog is empty or missing";
  }

  // read SPP message until #OK or #Error line
  std::string read_spp(double timeout = -1){
    if (!flt) throw Err() << "SPP: read from closed device";
    std::string ret;
    while (1){
      std::string l;

      // Read from SPP program with timeout.
      // Err is thrown if error happens.
      // Return -1 on EOF.
      int res = flt->getline(l, timeout);
      // line starts with the special character
      if (l.size()>0 && l[0] == ch){
        if (l.substr(1,7) == "Error: ") throw Err() << l.substr(8);
        if (l.substr(1,7) == "Fatal: ") throw Err() << l.substr(8);
        if (l.substr(1) == "OK") return ret;

        if (ret.size()>0) ret += '\n';
        if (l.size()>1 && l[1] == ch) ret += l.substr(1);
        else throw Err() << "SPP: symbol " << ch <<
          " in the beginning of a line is not protected: " << prog;
      }
      else {
        if (ret.size()>0) ret += '\n';
        ret += l;
      }
      if (res<0) break;
    }
    throw Err() << "SPP: no #OK or #Error message: " << prog;
  }

  // start SPP program
  void open() override {
    flt.reset(new IOFilter(prog));
    try {

      // first line: <symbol>SPP<version>
      std::string l;
      flt->getline(l, open_timeout);
      if (l.size()<5 || l[1]!='S' || l[2]!='P' || l[3]!='P')
        throw Err() << "SPP: unknown protocol, header expected: " << prog;
      ch  = l[0];
      int ver = str_to_type<int>(l.substr(4));
      if (ver!=1 && ver!=2) throw Err() << "SPP: unsupported protocol version: " << prog;
      read_spp(open_timeout); // ignore message, throw errors
    }
    catch (Err e) {
      close();
      throw e;
    }
  }

  // close SPP program
  void close() override {
    if (!flt) return;
    flt->close_input();
    flt->kill();
    flt.reset();
  }

  // send a message to the device and read answer
  std::string ask(const std::string & msg) override {
    if (!flt) throw Err() << "SPP: device is closed";
    flt->ostream() << msg << "\n";
    flt->ostream().flush();
    return read_spp(read_timeout);
  }
};

#include "tmc.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>

/*************************************************/
// usbtmc driver
// This driver supports devices connected via usbtmc kernel driver.
// It should work with all usual Agilent/Keysight devices connected
// via USB. Read timeout is set internally in the driver (5s?)
// but can be changed by -read_timeout option in the configuration file.
// Driver reads answer from the device only if there is a question mark '?'
// in the message.
// Options:
//  -dev -- serial device filename (e.g. /dev/usbtmc0)
//  -read_timeout  -- timeout for reading (default 0 - do not change)

struct Driver_usbtmc: Driver {
  std::string dev; // serial device name
  int fd;          // file descriptor
  double read_timeout; // read timeout, s

  Driver_usbtmc(const Opt & opts): Driver(opts) {
    dev = opts.get("dev");
    read_timeout = opts.get<double>("read_timeout", 0);
    if (dev == "") throw Err() << "Parameter -dev is empty or missing";
  }

  // open connection
  void open() override {
    fd = ::open(dev.c_str(), O_RDWR);
    if (fd<0) throw Err() << "usbtmc driver: can't open device: "
                          << dev << ": " << strerror(errno);

    // clear input and output buffers
    ssize_t ret = ioctl(fd, USBTMC_IOCTL_CLEAR, NULL);
    if (ret<0) throw Err() << "usbtmc driver: can't clear device state: "
                           << dev << ": " << strerror(errno);

    // set timeout (ms)
    if (read_timeout>0){
      uint32_t v = read_timeout*1000; // convert to ms
      ret = ioctl(fd, USBTMC_IOCTL_SET_TIMEOUT, &v);
      if (ret<0) throw Err() << "usbtmc driver: can't set timeout: "
                             << dev << ": " << strerror(errno);
    }
  }

  // close conection
  void close() override {
    ::close(fd);
  }

  // send a message to the device and read answer
  std::string ask(const std::string & msg) override {
    if (msg.size()==0) return "";
    // add '\n' if needed
    std::string m = msg;
    if (msg[msg.size()-1]!='\n') m+='\n';
    ssize_t ret = write(fd, m.data(), m.size());
    if (ret<0) throw Err() << "write error: " << strerror(errno);

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
      throw Err() << "usbtmc driver: can't read from "
                  << dev << ": " << strerror(en);
    }
    // remove '\n' is needed
    if (buf[ret-1]=='\n') ret--;
    return std::string(buf, buf+ret);
  }
};



#endif
