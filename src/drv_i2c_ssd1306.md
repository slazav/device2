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

### i2c commands

Every write should start with a control byte. If it is 0x80, then next
single byte is a command (or its argument), then next control byte can
follow. Example: to send command 0x81 and its argument 0xFF four
bytes should be sent: 0x80 0x81 0x80 0xFF. If control byte is 0x40, then
multiple data bytes are expected after it, until end of the write
transaction.

Reading gives a status byte. Datasheet specifies only bit6 for ON/OFF status.
For my 128x32 display it is 0x42 when off, 0x02 when on,
for 128x64 one it is 0x45/0x05.

Reading of display data is probably not supported for i2c interface.

Commands:

* 0x81 <value> -- set contrast 0..255
* 0xA4/0xA5 -- display shows memory content / entire display ON
* 0xA6/0xA7 -- normal / inverse display
* 0xAE/0xAF -- display OFF / display ON
* 0x26 0x00 <start_page> <interval> <end_page> 0x00 0xFF -- configure right horizontal scroll
* 0x27 0x00 <start_page> <interval> <end_page> 0x00 0xFF -- same for left horizontal scroll
* 0x29 0x00 <start_page> <interval> <end_page> <v_offset> -- vertical and right horizontal scroll
* 0x2A 0x00 <start_page> <interval> <end_page> <v_offset> -- vertical and left horizontal scroll
  - start_page, end_page: 0..7
  - interval:
    - 000 - 5 frames
    - 001 - 64 frames
    - 010 - 128 frames
    - 011 - 256 frames
    - 100 - 3 frames
    - 101 - 4 frames
    - 110 - 25 frames
    - 111 - 2 frames
  - vertical offset: 0..63
* 0x2E -- stop scrolling, RAM should be rewriten
* 0x2F -- start scrolling
* 0xA3 <N1> <N2> -- set vertical scroll area
  - N1 (0..63) - number of rows in the top fixed area, 
  - N2 (0..128) - number of rows in the scroll area
* 0x00 - 0x0F -- set lower column start for page addressing mode 0..15
* 0x10 - 0x1F -- set higher column start for page addressing mode 0..15 (0x10)
* 0x20 <val> -- set memory addressing mode
  - 00 -- horizontal
  - 01 -- vertical
  - 10 -- page
* 0x21 <start> <stop> -- set column address range: 0-127, 0-127
* 0x22 <start> <stop> -- set page address: 0-7, 0-7
* 0x40-0x7F -- set display start line (0..63 + 0x40)
* 0xA0/0xA1 -- horizontal swap
* 0xA8 <arg> -- set MUX ratio, 15..63
* 0xC0/0xC8 -- vertical swap
* 0xD3 <arg> -- set vertical offset (0..63)
* 0xDA 0x02|0x12|0x22|0x32 -- set pin hardware configuration (0x02 for 32-line display, 0x12 for 64-line display)
* 0xD5 <arg> -- set display clock divide ratio (lower 4 bits) and oscillateor frequency (higher 4 bits)
* 0xD9 <arg> -- set precharge period: phase 1 (1..15, lower 4 bits), phase2 (1..15, higher 4 bits)
* 0xDB 0x00|0x20|0x30 -- V_comh deselect level: 0.65Vcc, 0.77Vcc, 0.83Vcc
* 0xE3 -- no operation
