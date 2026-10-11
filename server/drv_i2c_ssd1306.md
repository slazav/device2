## Driver `i2c_ssd1306` -- SSD1306 display controller via i2c

This is a standard SSD1306 controller for 128x32, 128x64 OLED displays.

All i2c_* drivers can share a single i2c bus or a single device without collisions.

### Parameters:

* `-dev`     -- device path (e.g. /dev/i2c-1). Required.

* `-addr`    -- i2c address. Default 0x3C

* `-H`       -- set display height. Supported values: 32 and 64. Default: 64

* `-idn`     -- Output of *idn? command. Default: "SSD1306".

* `-errpref <str>` -- Prefix for error messages. Default: "i2c_ssd1306: "

### Commands introduced by the driver:

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
