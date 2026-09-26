#ifndef DRV_I2C_H
#define DRV_I2C_H

#include "drv.h"
#include "drv_utils.h"
#include "opt/opt.h"

/*************************************************/
/*
 * Driver `i2c` -- communication with linux i2c bus via text commands
 *

Tested with cp2112 usb-to-i2c bridge.

Driver works with a single i2c bus. Multiple devices can be attached to it.
The most basic i2c communication is supporter via single-line text commands:
`<address> [<bytes to write> ...] <read count>`
First <bytes to write> are sent to the device (if any), then <read count>
number of bytes are read (if non-zero).
All parameters are decimal or hexadecimal integers.
Don't forget to load i2c-dev kernel module.

Examples (assuming the device name in device2.cfg is i2c):

1. Get calibration parameters from BMP280 pressure sensor at address 0x76
by writing register number 0x8A and reading 24 bytes:

```
$ device_c ask i2c 0x76 0x8A 24
```

2. Adjust contrast of SSD1306-based OLED display (64 lines) at address
0x3C. The device requires bytes 0x80 to be written before command 0x81
and argument 127. No read is needed (zero bytes to read):

```
$ device_c ask i2c0 0x3C 0x80 0x81 0x80 127 0
```

Parameters:

* `-dev`     -- device path (e.g. /dev/i2c-1). Required.

* `-hex`     -- Output hexadecimal values instead of decimal (default: 0)

* `-idn`     -- Output of *idn? command. Default: "".

* `-errpref <str>` -- Prefix for error messages. Default: "i2c: "

*/

class Driver_i2c: public Driver {
protected:
  int fd;              // file descriptor
  std::string errpref; // error prefix
  std::string idn;
  bool hex;

public:

  Driver_i2c(const Opt & opts);
  ~Driver_i2c();

  std::string read() override { return "";}
  void write(const std::string & msg) override {}
  std::string ask(const std::string & msg) override;
};

#endif