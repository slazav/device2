## Driver `i2c` -- communication with linux i2c bus via text commands

Tested with cp2112 usb-to-i2c bridge.

Driver works with a single i2c bus. Multiple devices can be attached to it,
this driver and its children can use one bus without collisions.

Don't forget to load i2c-dev kernel module.


### Parameters

* `-dev`     -- i2c bus path (e.g. /dev/i2c-1). Required.

* `-addr`    -- i2c address. Required

* `-hex`     -- Output hexadecimal values instead of decimal for `-cmds i2c_text`. Default: 0

* `-idn`     -- Output of *idn? command. Default: "".

* `-errpref <str>` -- Prefix for error messages. Default: "i2c: "


### Commands introduced by the driver

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
