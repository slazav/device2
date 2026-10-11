#ifndef DRV_I2C_BMP280_H
#define DRV_I2C_BMP280_H

#include "drv_i2c.h"

class Driver_i2c_bmp280: public Driver_i2c {
protected:
  bool bme; // bmp280/bme280

public:
  Opt add_opts(const Opt & opts){
    opts.check_unknown({"dev", "errpref", "idn", "addr", "bme"});

    bme  = opts.get<bool>("bme", 0);

    // make options for i2c driver
    auto o = opts.clone_known({"dev", "addr", "errpref", "idn"});
    o.put_missing("errpref", "i2c_bmp280: ");
    o.put_missing("idn", "BMP280");
    o.put_missing<unsigned char>("addr", 0x76);
    return o;
  }


  Driver_i2c_bmp280(const Opt & opts): Driver_i2c(add_opts(opts)), cal_flag(false) {}
  ~Driver_i2c_bmp280() {}

  // read raw temperature / pressure
  uint32_t bmp280_raw(bool temp);

  // update calibration
  bool cal_flag; // was it updated
  double cal_data[12];
  void get_cal();

  std::string ask(const std::string & msg) override;
};

#endif
