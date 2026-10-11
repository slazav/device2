## Driver `i2c_bmp280` -- BMP280 pressure/temperature sensor via i2c

All i2c_* drivers can share a single i2c bus or a single device without collisions.

TODO: There is an extension for BME280 (+humidity measurements), but it's not fully supported yet.

### Parameters:

* `-dev`     -- device path (e.g. /dev/i2c-1). Required.

* `-addr`    -- i2c address. Default 0x76

* `-bme`     -- BME280, support for humidity readings

* `-idn`     -- Output of *idn? command. Default: "BMP280".

* `-errpref <str>` -- Prefix for error messages. Default: "i2c_bmp280: "

### Commands introduced by the driver:

* `id?` -- read id byte (0xD0), should be 0x58 for BMP280, 0x60 for BME280

* `status?` -- read status byte (0xF3):
  - bit0 -- im_update
  - bit3 -- measuring

* `ctrl_meas?` --  read ctrl_meas byte (0xF4)
   - bits 5-7 osrs_t -- Oversampling of temperature data
     - 000 - Skipped (output set to 0x80000)
     - 001 - oversampling 1
     - 010 - oversampling 2
     - 011 - oversampling 4
     - 100 - oversampling 8
     - 101, 110, 111 oversampling 16
   - bits 2-4 osrs_p -- Oversampling of pressure data
     - 000 Skipped (output set to 0x80000)
     - 001 - oversampling 1
     - 010 - oversampling 2
     - 011 - oversampling 4
     - 100 - oversampling 8
     - 101, 110, 111 - oversampling 16
   - bits 0-1 mode -- Power mode of the device

* `ctrl_meas <value>` -- set ctrl_meas byte

* `ctrl_hum?` -- read ctrl_hum byte (0xF2), BME280 only
  - bits 0-2 - osrs_h -- Controls oversampling of humidity data
    - 000 - Skipped (output set to 0x8000)
    - 001 - oversampling 1
    - 010 - oversampling 2
    - 011 - oversampling 4
    - 100 - oversampling 8
    - 101, 110, 111 oversampling 16

* `ctrl_hum <value>` -- set ctrl_meas byte (BME280 only)

* `config?` -- read config byte (0xF5)
  - bits 5-7, t_sb -- Controls inactive duration tstandby in normal mode.
  - bits 2-4, filter --  Controls the time constant of the IIR filter.
  - bit 0, spi3w_en -- Enables 3-wire SPI interface when set to 1.

* `config <value>` -- set config byte

* `init` -- quick init (set ctrl_meas 0xFF, config to 0x00)

* `reset` -- software reset

* `cal?` -- re-read and print calibration data.
  Normally calibration data is updated once, before the first measurement.
  If the device is changed while the driver is still running, calling `cal?`
  will update data.

* `meas? [<type>]` -- measure and print data
  type is "raw", "t_raw", "p_raw", "h_raw", "t", "p" or empty.

The command set is a bit random, improvements are welcome!
