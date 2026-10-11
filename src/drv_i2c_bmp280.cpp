#include "drv_i2c_bmp280.h"
#include "drv_utils.h"

#include <cstring>
#include "read_words/read_words.h"



void
Driver_i2c_bmp280::get_cal(){
  uint8_t reg = 0x88;
  i2c_write(&reg, 1);

  size_t n = 12;
  auto buf = i2c_read(2*n);
  auto b = (unsigned char *)buf.data();
  for (size_t i=0; i<n; i++){
    auto v = ((uint16_t)b[2*i+1] << 8) + (uint16_t)b[2*i];
    // values 0 and 3 are unsigned
    if (i==0 || i==3) cal_data[i] = v;
    else cal_data[i] = (int16_t)v;
  }


  cal_flag = true;
}


std::string
Driver_i2c_bmp280::ask(const std::string & msg) {

  // *idn? command
  if (idn.size() && strcasecmp(msg.c_str(),"*idn?")==0) return idn;


  std::istringstream in(unquote_words(msg));
  auto v = read_words(in, NULL, true, false); // read words, convert them to lower case
  if (v.size()==0) return std::string();

  // get lock
  auto lock = get_lock();

  // set address
  i2c_set_addr(addr);


  // read id byte (0xD0), should be 0x58 for BMP280, 0x60 for BME280
  if (v[0] == "id?"){
    uint8_t reg = 0xD0;
    i2c_write(&reg, 1);
    auto buf = i2c_read(1);
    return type_to_str_hex((uint8_t)buf[0]);
  }

  // read status byte (0xF3)
  // - bit0 -- im_update
  // - bit3 -- measuring
  if (v[0] == "status?"){
    uint8_t reg = 0xF3;
    i2c_write(&reg, 1);
    auto buf = i2c_read(1);
    return type_to_str_hex((uint8_t)buf[0]);
  }

  // read ctrl_meas byte (0xF4)
  // - bits 5-7 osrs_t -- Oversampling of temperature data
  //   - 000 - Skipped (output set to 0x80000)
  //   - 001 - oversampling 1
  //   - 010 - oversampling 2
  //   - 011 - oversampling 4
  //   - 100 - oversampling 8
  //   - 101, 110, 111 oversampling 16
  // - bits 2-4 osrs_p -- Oversampling of pressure data
  //   - 000 Skipped (output set to 0x80000)
  //   - 001 - oversampling 1
  //   - 010 - oversampling 2
  //   - 011 - oversampling 4
  //   - 100 - oversampling 8
  //   - 101, 110, 111 - oversampling 16
  // - bits 0-1 mode -- Power mode of the device
  if (v[0] == "ctrl_meas?"){
    uint8_t reg = 0xF4;
    i2c_write(&reg, 1);
    auto buf = i2c_read(1);
    return type_to_str_hex((uint8_t)buf[0]);
  }

  // set ctrl_meas byte
  if (v[0] == "ctrl_meas"){
    if (v.size()!=2) throw Err() << "ctrl_meas: argument expected";
    const unsigned char data[] = {0xF4, str_to_type<uint8_t>(v[1])};
    i2c_write(data, sizeof(data));
    return std::string();
  }

  // read ctrl_hum byte
  // - bits 0-2 - osrs_h -- Controls oversampling of humidity data
  //   - 000 - Skipped (output set to 0x8000)
  //   - 001 - oversampling 1
  //   - 010 - oversampling 2
  //   - 011 - oversampling 4
  //   - 100 - oversampling 8
  //   - 101, 110, 111 oversampling 16
  if (bme && v[0] == "ctrl_hum?"){ // BME280
    uint8_t reg = 0xF2;
    i2c_write(&reg, 1);
    auto buf = i2c_read(1);
    return type_to_str_hex((uint8_t)buf[0]);
  }

  // set ctrl_hum byte (BME280 only)
  if (bme && v[0] == "ctrl_hum"){
    if (v.size()!=2) throw Err() << "ctrl_hum: argument expected";
    const unsigned char data[] = {0xF2, str_to_type<uint8_t>(v[1])};
    i2c_write(data, sizeof(data));
    return std::string();
  }

  // read config byte (0xF5)
  // - bits 5-7, t_sb -- Controls inactive duration tstandby in normal mode.
  // - bits 2-4, filter --  Controls the time constant of the IIR filter.
  // - bit 0, spi3w_en -- Enables 3-wire SPI interface when set to 1.
  if (v[0] == "config?"){
    uint8_t reg = 0xF5;
    i2c_write(&reg, 1);
    auto buf = i2c_read(1);
    return type_to_str_hex((uint8_t)buf[0]);
  }

  // set config byte
  if (v[0] == "config"){
    if (v.size()!=2) throw Err() << "config: argument expected";
    const unsigned char data[] = {0xF5, str_to_type<uint8_t>(v[1])};
    i2c_write(data, sizeof(data));
    return std::string();
  }

  // quick init (set ctrl_meas 0xFF, config to 0x00)
  if (v[0] == "init"){
    const unsigned char data[] = {0xF4,0xFF,0xF5,0x00};
    i2c_write(data, sizeof(data));
    return std::string();
  }

  // software reset
  if (v[0] == "reset"){
    uint8_t reg = 0xB6;
    i2c_write(&reg, 1);
    return std::string();
  }

  // re-read and print calibration data
  if (v[0] == "cal?"){
    get_cal();
    std::string ret;
    for (size_t i=0; i<12; i++){
      if (i>0) ret += " ";
      ret += type_to_str((int)cal_data[i]);
    }
    return ret;
  }

  // read temperature
  // read? [<type>], type is T_raw, P_raw, raw, T, P
  if (v[0] == "meas?"){
    std::string type;
    if (v.size()>1) type = v[1];

    // get both temperature and pressure in a single read
    uint8_t reg = 0xF7;
    i2c_write(&reg, 1);
    auto buf = i2c_read(bme? 8:6);
    auto b = (unsigned char *)buf.data();

    double p_raw = ((uint32_t)b[0]<<12) + ((uint32_t)b[1]<<4) + ((uint32_t)b[2]>>4);
    double t_raw = ((uint32_t)b[3]<<12) + ((uint32_t)b[4]<<4) + ((uint32_t)b[5]>>4);
    double h_raw = bme? (((uint32_t)b[6]<<8) + (uint32_t)b[7]) : 0;

    if (type=="t_raw") return type_to_str((uint32_t)t_raw);
    if (type=="p_raw") return type_to_str((uint32_t)p_raw);
    if (type=="h_raw" && bme) return type_to_str((uint32_t)h_raw);
    if (type=="raw") return type_to_str((uint32_t)t_raw)
                          + " " + type_to_str((uint32_t)p_raw)
                          + (bme? (" " + type_to_str((uint32_t)h_raw)) : "");

    if (!cal_flag) get_cal(); // update calibration if needed

    // convert temperature
    double var1 = t_raw/16384.0 - cal_data[0]/1024.0;
    double t_fine = var1*cal_data[1] + var1*var1/64.0*cal_data[2];
    double T = t_fine/5120.0;

    double var2;
    if (type=="t") return type_to_str(T);
    if (type=="t_fine") return type_to_str(t_fine);

    var1 = (t_fine/2.0) - 64000.0;
    var2 = var1 * (var1 * cal_data[8]/32768.0);
    var2 = var2 + (var1 * cal_data[7] * 2.0);
    var2 = (var2/4.0)+(cal_data[6]*65536.0);
    var1 = (cal_data[5]*var1*var1/524288.0 + cal_data[4]*var1) / 524288.0;
    var1 = (1.0 + var1/32768.0) * cal_data[3];
    double P = 1048576.0 - p_raw;
    P = (P - (var2/4096.0))*6250.0/var1;
    var1 = cal_data[11]*P*P/2147483648.0;
    var2 = P*cal_data[10]/32768.0;
    P = P + (var1+var2+cal_data[9])/16.0;

    if (type=="p") return type_to_str(P);
    return type_to_str(T) + " " + type_to_str(P);
  }

  throw Err() << "unknown command: " << msg;
};
