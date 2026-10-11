#ifndef DRV_I2C_SSD1306_H
#define DRV_I2C_SSD1306_H

#include "drv_i2c.h"

/*************************************************/
/*
 * Driver `i2c_ssd1306` -- i2c communication with display controller SSD1306
 *

This is a standard SSD1306 controller for 128x32, 128x64 OLED displays.

All i2c_* drivers can share a single i2c bus or a single device without collisions.

Parameters:

* `-dev`     -- device path (e.g. /dev/i2c-1). Required.

* `-addr`    -- i2c address. Default 0x3C

* `-H`       -- set display height. Supported values: 32 and 64. Default: 64

* `-idn`     -- Output of *idn? command. Default: "SSD1306".

* `-errpref <str>` -- Prefix for error messages. Default: "i2c_ssd1306: "

Commands introduced bu the driver:

* `init` -- quick init
* `on (1|0)` -- switch display on and off
* `white (1|0)` -- switch display white and back
* `inv (1|0)` -- switch display to inverse mode and back
* `hflip (1|0)` -- horizontal flip
* `vflip (1|0)` -- vertical flip
* `contrast <value>` -- set contrast to 0..255
* `clear` -- clear display
* `puts <text> ...` -- write a string (using some 256-bit koi8_r 8x8 font)
* `crnl` -- carrage return + new line
* `goto <x> <y>` -- set cursor position

The command set is a bit random, improvements are welcome!

*/

class Driver_i2c_ssd1306: public Driver_i2c {
protected:
  uint8_t W, H; // display dimensions (128x64 or 128x32)
  uint8_t pin_conf; // pin configuration setting (usually 0x02 for 128x32 and 0x12 for 128x64)
  unsigned char x,y; //cursor position (used in `puts` command)

public:

  // Prepare options for passing to Driver_i2c constructor.
  // set some driver-specific parameters.
  Opt add_opts(const Opt & opts){
    opts.check_unknown({"dev", "addr", "H", "W", "errpref", "idn"});

    //display dimensions add related values
    W = 128;
    H = opts.get<unsigned char>("H", 64);
    if (H!=32 && H!=64) throw Err() << "unsupported value for -H: " << H;
    pin_conf = H==32? 0x02:0x12;
    x=y=0;

    // make options for i2c driver
    auto o = opts.clone_known({"dev", "addr", "errpref", "idn"});
    o.put_missing("errpref", "i2c_ssd1306: ");
    o.put_missing("idn", "SSD1306");
    o.put_missing<unsigned char>("addr", 0x3C);
    return o;
  }


  Driver_i2c_ssd1306(const Opt & opts): Driver_i2c(add_opts(opts)) {}
  ~Driver_i2c_ssd1306() {}

  // send multiple commands
  void write_cmds(unsigned char *data, size_t len);

  // put single character
  void putc(uint8_t x, uint8_t y, uint8_t c);

  // put string at the cursor position
  void puts(const std::string & s);

  std::string ask(const std::string & msg) override;
};

#endif
