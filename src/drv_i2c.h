#ifndef DRV_I2C_H
#define DRV_I2C_H

#include "drv.h"
#include "drv_utils.h"
#include "opt/opt.h"

#include <sys/types.h> // dev_t
#include <map>
#include <mutex>
#include <memory>
#include <thread>

class Driver_i2c: public Driver {
protected:
  std::string dev;     // linux i2d device name
  int fd;              // file descriptor
  dev_t st_dev;        // Linux device ID
  unsigned char addr;  // i2c address
  std::string errpref; // error prefix
  std::string idn;
  bool hex;


public:

  // Mutexes for each device
  static std::map<dev_t, std::unique_ptr<std::mutex> > mtxs;

  // Get lock for device (for writers)
  std::unique_lock<std::mutex> get_lock() {
    // create mutex if it does not exist yet:
    if (mtxs.count(st_dev)==0) mtxs.emplace(st_dev, new std::mutex);
    return std::unique_lock<std::mutex>(*mtxs[st_dev]);
  }

  Driver_i2c(const Opt & opts);
  ~Driver_i2c();

  // set i2c address
  void i2c_set_addr(unsigned char addr);

  // write multiple bytes to i2c bus
  void i2c_write(const unsigned char data[], const size_t size);
  void i2c_write(const std::string & buf){
    i2c_write((const unsigned char*)buf.data(), buf.size());}

  // read multiple bus from i2c bus
  std::string i2c_read(size_t count);

  std::string ask(const std::string & msg) override;
};

#endif
