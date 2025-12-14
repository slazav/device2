#include <cstring>
#include <cstdint>
#include <cmath>
#include <sstream>

#include "read_words/read_words.h"
#include "drv_serial_hm310t.h"


//see https://github.com/LacobusVentura/MODBUS-CRC16
uint16_t
Driver_serial_hm310t::modbus_crc(const uint8_t *buf, size_t len ){
  static const uint16_t table[2] = { 0x0000, 0xA001 };
  uint16_t crc = 0xFFFF;
  unsigned int x = 0;

  for(size_t i = 0; i<len; i++){
    crc ^= buf[i];
    for (uint8_t bit = 0; bit < 8; bit++){
      x = crc & 0x01;
      crc >>= 1;
      crc ^= table[x];
    }
  }
  return crc;
}


uint16_t
Driver_serial_hm310t::modbus_func3(const uint16_t addr){

  int fcode=3;

  // create modbus request and send via serial port
  std::string msg(8,0);
  msg[0] = dev_addr; // slave address (0 - broadcast)
  msg[1] = fcode;    // function code
  msg[2] = (addr >> 8);   // address H
  msg[3] = (addr & 0xFF); // address L
  msg[5] = 1;    // count L
  auto crc = modbus_crc((uint8_t*)msg.data(), 6);
  msg[6] = (crc & 0xFF); // crc L
  msg[7] = (crc >> 8);    // crc H
  Driver_serial::write(msg);

  // read response: <slave> <fcode> <count> <data[count]> <crc[2]>
  // or error: <slave> <fcode+0x80> <exception> <crc[2]>
  auto res = Driver_serial::read();
  if (res.size()<2) throw Err() << errpref << "short response";

  Err e;
  uint8_t fc = res[1];
  e << errpref << "modbus error: function code: " << (int)fc << ": ";
  // Process errors
  if (fc == 0x80 + fcode){
    if (res.size()!=5) throw e << "can't parse error response, wrong length: " << res.size();
    e << "exception code: " << (int)res[2];
    switch (res[2]) {// exception code
      case 1: throw e << " (unsupported function code)";
      case 2: throw e << " (wrong address)";
      case 3: throw e << " (wrong count)";
      case 4: throw e << " (error during request processing)";
    }
    throw e << " (unknown exception code)";
  }

  if (fc != fcode) throw e << "unknown function code";
  if (res.size()!=7) throw e << "unexpected response length";
  if (res[2]!=2) throw e << "unexpected response count";
  return  ((uint16_t)(uint8_t)res[3]<<8) + (uint8_t)res[4];
}

void
Driver_serial_hm310t::modbus_func6(const uint16_t addr, const uint16_t val){

  int fcode=6;

  // create modbus request and send via serial port
  std::string msg(8,0);
  msg[0] = dev_addr; // slave address (0 - broadcast)
  msg[1] = fcode;    // function code
  msg[2] = (addr >> 8);   // address H
  msg[3] = (addr & 0xFF); // address L
  msg[4] = (val >> 8);    // value H
  msg[5] = (val & 0xFF);  // value L
  auto crc = modbus_crc((uint8_t*)msg.data(), 6);
  msg[6] = (crc & 0xFF); // crc L
  msg[7] = (crc >> 8);    // crc H
  Driver_serial::write(msg);

  // read response: <slave> <fcode> <addr> <value> <crc[2]> (echo of the request)
  // or error: <slave> <fcode+0x80> <exception> <crc[2]>
  auto res = Driver_serial::read();
  if (res.size()<2) throw Err() << errpref << "short response";

  Err e;
  uint8_t fc = res[1];
  e << errpref << "modbus error: function code: " << (int)fc << ": ";
  // Process errors
  if (fc == 0x80 + fcode){
    if (res.size()!=5) throw e << "can't parse error response, wrong length: " << res.size();
    e << "exception code: " << (int)res[2];
    switch (res[2]) {// exception code
      case 1: throw e << " (unsupported function code)";
      case 2: throw e << " (wrong value)";
      case 3: throw e << " (wrong count)";
      case 4: throw e << " (error during request processing)";
    }
    throw e << " (unknown exception code)";
  }

  if (fc != fcode) throw e << "unknown function code";
  if (res != msg) throw e << "unexpected response";
}

void
Driver_serial_hm310t::read_dpt(){
  auto dpt = modbus_func3(0x05);
  Ve = (dpt >> 8) & 0xF;
  Ie = (dpt >> 4) & 0xF;
  Pe = dpt & 0xF;
}

std::string
Driver_serial_hm310t::read_volt(const uint16_t addr){
  return type_to_str(pow(0.1,Ve)*modbus_func3(addr)); }

std::string
Driver_serial_hm310t::read_curr(const uint16_t addr){
  return type_to_str(pow(0.1,Ie)*modbus_func3(addr));
}

std::string
Driver_serial_hm310t::read_pwr(const uint16_t addrH, const uint16_t addrL){
  auto pwr = (uint32_t)modbus_func3(addrH)<<16 + modbus_func3(addrL);
  return type_to_str(pow(0.1,Pe)*pwr);
}

void
Driver_serial_hm310t::write_volt(const uint16_t addr, const std::string & arg){
  modbus_func6(addr, pow(10,Ve)*str_to_type<double>(arg));
}

void
Driver_serial_hm310t::write_curr(const uint16_t addr, const std::string & arg){
  modbus_func6(addr, pow(10,Ie)*str_to_type<double>(arg));
}

void
Driver_serial_hm310t::write_pwr(const uint16_t addrH, const uint16_t addrL, const std::string & arg){
  uint32_t pwr = pow(10,Pe)*str_to_type<double>(arg);
  modbus_func6(addrH, pwr>>16);
  modbus_func6(addrL, pwr & 0xFFFF);
}

std::string
Driver_serial_hm310t::ask(const std::string & msg) {
  if (strcasecmp(msg.c_str(),"*idn?")==0) return idn;

  // output status (0|1)
  if (strcasecmp(msg.c_str(),"out?")==0) return type_to_str(modbus_func3(0x01));
  // protection status mask (raw data)
  if (strcasecmp(msg.c_str(),"stat:raw?")==0) return type_to_str(modbus_func3(0x02));

  // protection status in human-readabe form (could be incomplete)
  if (strcasecmp(msg.c_str(),"stat?")==0){
    auto v = modbus_func3(0x02);
    std::string ret;
    if (v & (1<<2)) ret += " OVP";
    if (v & (1<<3)) ret += " OCP";
    if (v & (1<<4)) ret += " OPP";
    if (v & (1<<5)) ret += " OTP";
    if (v & (1<<6)) ret += " SCP";
    return ret;
  }
  // specification and type (?)
  if (strcasecmp(msg.c_str(),"spec:raw?")==0) return type_to_str(modbus_func3(0x03));

  // tail classification (?)
  if (strcasecmp(msg.c_str(),"tail:raw?")==0) return type_to_str(modbus_func3(0x04));

  // decimal point position (raw data)
  if (strcasecmp(msg.c_str(),"dpt:raw?")==0) return type_to_str(modbus_func3(0x05));

  // parse decimal point position (voltage, current, power)
  if (strcasecmp(msg.c_str(),"dpt?")==0){
    read_dpt(); // update values from device
    std::ostringstream ret;
    ret << Ve << " " << Ie << " " << Pe;
    return ret.str();
  }
  // measured voltage [V]
  if (strcasecmp(msg.c_str(),"volt:meas?")==0) return read_volt(0x10);
  // measured current [A]
  if (strcasecmp(msg.c_str(),"curr:meas?")==0) return read_curr(0x11);
  // measured power [W]
  if (strcasecmp(msg.c_str(),"pwr:meas?")==0) return read_pwr(0x12, 0x13);
  // set voltage [V]
  if (strcasecmp(msg.c_str(),"volt?")==0) return read_volt(0x30);
  // set current [A]
  if (strcasecmp(msg.c_str(),"curr?")==0) return read_curr(0x31);
  // over voltage protection [V]
  if (strcasecmp(msg.c_str(),"ovp?")==0) return read_volt(0x20);
  // over current protection [A]
  if (strcasecmp(msg.c_str(),"ocp?")==0) return read_curr(0x21);
  // over power protection [W]
  if (strcasecmp(msg.c_str(),"opp?")==0) return read_pwr(0x22, 0x23);
  // address
  if (strcasecmp(msg.c_str(),"addr?")==0) return type_to_str(modbus_func3(0x9999));

  // operations with arguments, without answer
  Driver_serial_hm310t::write(msg);
  return std::string();
}

void
Driver_serial_hm310t::write(const std::string & msg) {
  std::istringstream in(unquote_words(msg));
  auto v = read_words(in, NULL, true, false); // read words, convert them to lower case
  if (v.size()==0) return;

  if (v[0] == "out"){
    if (v.size()!=2) throw Err() << errpref << "argument expected: " << msg;
    return modbus_func6(0x01, str_to_type<bool>(v[1]));
  }
  if (v[0] == "volt"){
    if (v.size()!=2) throw Err() << errpref << "argument expected: " << msg;
    return write_volt(0x30, v[1]);
  }
  if (v[0] == "curr"){
    if (v.size()!=2) throw Err() << errpref << "argument expected: " << msg;
    return write_curr(0x31, v[1]);
  }
  if (v[0] == "ovp"){
    if (v.size()!=2) throw Err() << errpref << "argument expected: " << msg;
    return write_volt(0x20, v[1]);
  }
  if (v[0] == "ocp"){
    if (v.size()!=2) throw Err() << errpref << "argument expected: " << msg;
    return write_curr(0x21, v[1]);
  }
  if (v[0] == "opp"){
    if (v.size()!=2) throw Err() << errpref << "argument expected: " << msg;
    return write_pwr(0x22, 0x23, v[1]);
  }

  // We can't query different device addresses (and it's useless for USB), do not change it either:
  // if (v[0] == "addr") return modbus_func6(0x01, str_to_type<uint16_t>(v[1]));

  throw Err() << errpref << "unsupported command: " << msg;
}
