#include "drv_i2c_ssd1306.h"
#include "drv_utils.h"

#include <cstring>
#include "read_words/read_words.h"

#include "./fonts/font_koi8_r.inc"

void
Driver_i2c_ssd1306::write_cmds(unsigned char *data, size_t len){
  // each byte should be preceeded with 0x80
  std::string buf(len*2, '\0');
  for (size_t i=0; i<len; i++){
    buf[2*i] = 0x80;
    buf[2*i+1] = data[i];
  }
  i2c_write(buf);
}

void
Driver_i2c_ssd1306::putc(uint8_t x, uint8_t y, uint8_t c){
  if (x>W/8 || y>H/8) return;

  // set update region
  unsigned char data[] = {
    0x21, (uint8_t)(x*8), 0x7F, //column range 0..127
    0x22, y, 7, //page range 0..7
  };
  write_cmds(data, sizeof(data));
  i2c_write(display_font_koi8_r[c], 9);
}

void
Driver_i2c_ssd1306::puts(const std::string & s){
  for (size_t i=0; i<s.size(); i++){
    if (s[i] == '\\'){
      i++;
      if (i>=s.size()) break;
      if (s[i] == 'n'){
        x = 0;
        y = (y+1)%(W/8);
        continue;
      }
    }
    putc(x, y, s[i]);

    // move cursor
    if (x == W/8 - 1) y = (y+1)%(W/8);
    x = (x+1)%(W/8);
  }
}


std::string
Driver_i2c_ssd1306::ask(const std::string & msg) {

  // *idn? command
  if (idn.size() && strcasecmp(msg.c_str(),"*idn?")==0) return idn;

  std::istringstream in(unquote_words(msg));
  auto v = read_words(in, NULL, true, false); // read words, convert them to lower case
  if (v.size()==0) return std::string();

  // get lock
  auto lock = get_lock();

  // set i2c address
  i2c_set_addr(addr);

  // `init` -- quick init
  if (v[0] == "init"){
    uint8_t pin_conf = H==32? 0x02:0x12;
    uint8_t data[] = {
      0xAE,       // display off
      0x20, 0x00, // memory address mode (0 - horizontal, 1 - vertical, 2 - page)
      0x40,       // display start line = 0
      0xA1,       // segment re-map, column 127 is mapped to SEG0
      0xA8, (unsigned char)(H-1),  // set multiplex ratio
      0xC8,       // set COM (common) output scan direction. Scan from bottom up, COM[N-1] to COM0
      0xD3, 0x00, // set display offset
      0xDA, pin_conf, // set common pins hardware configuration.  0x02 for 128x32, 0x12 for 128x64
      0xD5, 0x80, // display clock divide ratio (0x80 - div ratio of 1, standard freq)
      0xD9, 0xF1, // pre-charge periods
      0xDB, 0x30, // VCOMH deselect level (0.83xVcc)
      0x81, 0x7F, // contrast
      0xA4,       // display follows RAM content
      0xA6,       // non-inverted display
      0x8D, 0x14, // set charge pump (Vcc internally generated on our board)
      0x2E,       // deactivate scrolling if set
      0xAF        // display on
    };
    write_cmds(data, sizeof(data));
    x=y=0;
    return std::string();
  }

  // `on (1|0)` -- switch display on and off
  if (v[0] == "on"){
    bool arg = v.size()>1 ? str_to_type<bool>(v[1]) : true;
    uint8_t byte = 0xAE + arg;
    write_cmds(&byte, 1);
    return std::string();
  }

  // `white (1|0)` -- switch display white and back
  if (v[0] == "white"){
    bool arg = v.size()>1 ?  str_to_type<bool>(v[1]) : true;
    uint8_t byte = 0xA4 + arg;
    write_cmds(&byte, 1);
    return std::string();
  }

  // `inv (1|0)` -- switch display to inverse mode and back
  if (v[0] == "inv"){
    bool arg = v.size()>1 ?  str_to_type<bool>(v[1]) : true;
    uint8_t byte = 0xA6 + arg;
    write_cmds(&byte, 1);
    return std::string();
  }

  // `hflip (1|0)` -- horizontal flip
  if (v[0] == "hflip"){
    bool arg = v.size()>1 ?  str_to_type<bool>(v[1]) : true;
    uint8_t byte = 0xA0 + !arg;
    write_cmds(&byte, 1);
    return std::string();
  }

  // `vflip (1|0)` -- vertical flip
  if (v[0] == "vflip"){
    bool arg = v.size()>1 ?  str_to_type<bool>(v[1]) : true;
    uint8_t byte = arg ? 0xC0:0xC8;
    write_cmds(&byte, 1);
    return std::string();
  }

  // `contrast <value>` -- set contrast to 0..255
  if (v[0] == "contrast"){
    if (v.size()!=2) throw Err() << "contrast: one argument expected";
    auto arg = str_to_type<uint8_t>(v[1]);
    uint8_t data[] = {0x81, arg};
    write_cmds(data, sizeof(data));
    return std::string();
  }

  // `clear` -- clear display
  if (v[0] == "clear"){
    // set range
    unsigned char data[] = {
      0x21, 0x00, (uint8_t)(W-1), // column range 0..127
      0x22, 0x00, (uint8_t)(H/8-1) // page range 0..7
    };
    write_cmds(data, sizeof(data));

    // With CP2112 there is a 60 byte limit for writes
    // (This is not a limit of SD1306, from MCU I was able to write all memory at once).
    // We need to write W*H/8 = 512 or 1024 bytes + 1 control byte in each block:
    size_t num = (size_t)W*H/8;
    for (size_t i = 0; i<num/32; i+=1){
      std::string buf(1 + 32, (char)0);
      buf[0]= (char)0x40;
      i2c_write(buf);
    }
    x = y = 0;
    return std::string();
  }

  // `puts <text> ...` -- write a string
  if (v[0] == "puts"){
    if (msg.size()<=5) throw Err() << "text expected: " << msg;
    puts(msg.substr(5));
    return std::string();
  }

  // `crnl` -- carrage return + new line
  if (v[0] == "crnl"){
    x = 0;
    y = (y+1)%(W/8);
  }

  // `goto <x> <y>` -- set cursor position
  if (v[0] == "goto"){
    if (v.size()!=3) throw Err() << "two arguments expected: " << msg;
    x = str_to_type<uint8_t>(v[1]);
    y = str_to_type<uint8_t>(v[2]);
    return std::string();
  }

  throw Err() << "unknown command: " << msg;
};
