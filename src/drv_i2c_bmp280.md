## Driver `i2c_bmp280` -- BMP280/BME280 pressure/temperature/humidity sensor via i2c

All i2c_* drivers can share a single i2c bus or a single device without collisions.

TODO: Humidity calibration for BME280 is not implemented

### Parameters:

* `-dev`     -- device path (e.g. /dev/i2c-1). Required.

* `-addr`    -- i2c address. Default 0x76

* `-bme`     -- BME280, support for humidity readings

* `-idn`     -- Output of *idn? command. Default: "BMP280".

* `-errpref <str>` -- Prefix for error messages. Default: "i2c_bmp280: "

### Commands introduced by the driver:

* `id?` -- read id byte 0xD0, should be 0x58 for BMP280, 0x60 for BME280
* `status?` -- read status byte 0xF3:
* `ctrl_meas?` --  read ctrl_meas byte 0xF4
* `ctrl_meas <value>` -- set ctrl_meas byte
* `ctrl_hum?` -- read ctrl_hum byte 0xF2, (BME280 only)
* `ctrl_hum <value>` -- set ctrl_meas byte (BME280 only)
* `config?` -- read config byte 0xF5
* `config <value>` -- set config byte
* `init` -- quick init (set ctrl_meas 0xFF, config to 0x00, continuous measurement with maxinin oversampling)
* `reset` -- software reset

* `cal?` -- re-read and print calibration data.
  Normally calibration data is updated once, before the first measurement.
  If the device is changed while the driver is still running, calling `cal?`
  will update data.

* `meas? [<type>]` -- measure and print data
  type is "raw", "t_raw", "p_raw", "h_raw", "t", "p" or empty.

The command set is a bit random, improvements are welcome!

### i2c registers

Registers for BMP280 and BME280

* 0x88-0xA1: `calib00 - calib25` -- Calibration data
  - 0x88/0x89 - `dig_T1` (uint16_t)
  - 0x8A/0x8B - `dig_T2` (int16_t)
  - 0x8C/0x8D - `dig_T3` (int16_t)
  - 0x8E/0x8F - `dig_P1` (uint16_t)
  - 0x90/0x91 - `dig_P2` (int16_t)
  - 0x92/0x93 - `dig_P3` (int16_t)
  - 0x94/0x95 - `dig_P4` (int16_t)
  - 0x96/0x97 - `dig_P5` (int16_t)
  - 0x98/0x99 - `dig_P6` (int16_t)
  - 0x9A/0x9B - `dig_P7` (int16_t)
  - 0x9C/0x9D - `dig_P8` (int16_t)
  - 0x9E/0x9F - `dig_P9` (int16_t)
  - BME280: 0xA1 - `dig_H1` (uint8_t)
  - BME280: 0xE1/0xE2 - `dig_H2` (int16_t)
  - BME280: 0xE3  - `dig_H3` (uint8_t), BME280 only
  - BME280: 0xE4/0xE5[3:0] - `dig_H4` (int16_t)
  - BME280: 0xE5[7:4]/0xE6 - `dig_H5` (int16_t)

* 0xD0: `id`
  - BMP280: 0x58
  - BME280: 0x60

* 0xE0: `reset`
  - write 0xB6 to do software reset

* BME280: 0xF2 - `ctrl_hum`
  - bits 0-2: `osrs_h` -- Oversampling of humidity data
    - 000 Skipped (output set to 0x8000)
    - 001 - oversampling 1
    - 010 - oversampling 2
    - 011 - oversampling 4
    - 100 - oversampling 8
    - 101, 110, 111 - oversampling 16

* 0xF3: `status`
  - bit 0: im_update
  - bit 3: measuring

* 0xF4: `ctrl_meas`
  - bits 5-7: `osrs_t` -- Oversampling of temperature data
    - 000 - Skipped (output set to 0x80000)
    - 001 - oversampling 1 (16bit, 0.005C resolution)
    - 010 - oversampling 2 (17bit, 0.0025C)
    - 011 - oversampling 4 (18bit, 0.0012C)
    - 100 - oversampling 8 (19bit, 0.0006C)
    - 101, 110, 111 oversampling 16 (20bit, 0.0003C)
  - bits 2-4: `osrs_p` -- Oversampling of pressure data
    - 000 Skipped (output set to 0x80000)
    - 001 - oversampling 1 (16bit, 2.62Pa resolution)
    - 010 - oversampling 2 (17bit, 1.31Pa)
    - 011 - oversampling 4 (18bit, 0.66Pa)
    - 100 - oversampling 8 (19bit, 0.33Pa)
    - 101, 110, 111 - oversampling 16 (20bit, 0.16Pa)
  - bits 0-1: `mode` -- Power mode of the device
    - 00 -- sleep mode, default, no measurements
    - 01,10 -- forced mode, do single measurement and return to sleep mode
    - 11 -- normal mode -- repeating measurements with t_sb delay between them

* 0xF5: `config`
  - bit 0: `spi3w_en` -- Enables 3-wire SPI interface when set to 1
  - bits 2-4: `filter` -- Time constant of the IIR filter.
    - 000 - K=1, filter off
    - 001 - K=2
    - 010 - K=4
    - 011 - K=8
    - 100,101,110,111 - K=16
  - bits 5-7: `t_sb` -- Inactive duration in normal mode
    - 000 - 0.5ms
    - 001 - 62.5ms
    - 010 - 125ms
    - 011 - 250ms
    - 100 - 500ms
    - 101 - 1s
    - 110 - BMP280: 2s, BME280: 10ms
    - 111 - BMP280: 4s, BME280: 20ms

* 0xF7,0xF8,0xF9 -- `press_msb, press_lsb, press_xlsb`
* 0xFA,0xFB,0xFC -- `temp_msb, temp_lsb, temp_xlsb`
* BME280: 0xFD,0xFE -- `hum_msb, hum_lsb`

For both temperature and pressure raw ADC value can be obtained as (msb<<12) + (lsb<<4) + (xlsb>>4).
It is recommended to read all three bytes in a single i2c transaction.

Filtered data is calculated as V_new = (V_old*(K-1) + V_adc)/K.
Filter is reset when writing to `config` register

Calibration functions can be found in the datasheets.
