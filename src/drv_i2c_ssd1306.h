#ifndef DRV_I2C_SSD1306_H
#define DRV_I2C_SSD1306_H

#include "drv_i2c.h"

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
