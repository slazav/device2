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

/*************************************************/
/*
 * Driver `i2c` -- communication with linux i2c bus via text commands
 *

Tested with cp2112 usb-to-i2c bridge.

Driver works with a single i2c bus. Multiple devices can be attached to it,
this driver and its children can use one bus without collisions.

Don't forget to load i2c-dev kernel module.


Parameters:

* `-dev`     -- i2c bus path (e.g. /dev/i2c-1). Required.

* `-addr`    -- i2c address. Required

* `-hex`     -- Output hexadecimal values instead of decimal for `-cmds i2c_text`. Default: 0

* `-idn`     -- Output of *idn? command. Default: "".

* `-errpref <str>` -- Prefix for error messages. Default: "i2c: "


Commands:

* `write <byte1> ...` -- write multiple bytes to the device
* `read? <num>` -- read `<num>` bytes from the device

Each line can have a few read and write commands.
All parameters are decimal or hexadecimal integers.

Examples (assuming the device name in device2.cfg is mydev):

1. Get calibration parameters from BMP280 pressure sensor
by writing register number 0x8A and reading 24 bytes:

```
$ device_c ask mydev write 0x8A read 24
```

2. Set contrast of SSD1306-based OLED display to 127 by
writing 4-byte command sequence, no reading:

```
$ device_c ask mydev write 0x80 0x81 0x80 127
```

*/

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
