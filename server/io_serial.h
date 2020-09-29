#ifndef IO_SERIAL_H
#define IO_SERIAL_H

/*************************************************/
/*
 * Access to a serial device
 *
 * See https://www.cmrr.umn.edu/~strupp/serial.html

Options:
  -dev     -- Serial device filename (e.g. /dev/ttyUSB0)
              Required.

  -ndelay  -- Non-blocking mode (0|1)
              Default: do not set.

  -parity  -- Parity (8N1, 7N1, 7O1, 7S1).
              Default: do not set.

  -baud    -- Baud rate (both input and output), integer value
              posix values: 0,50,75,110,134,150,200,300,600,1200,
                1800,2400,4800,9600,19200,38400.
              non-posix falues: 57600, 115200, 230400, 460800,
                500000, 576000, 921600, 1000000, 1152000, 1500000,
                2000000, 2500000, 3000000, 3000000, 3500000, 4000000.
              Default: do not set.

  -raw     -- Raw/canonical input (0|1).
              Default: do not set.

  -sfc     -- Enable/disable software flow control (0|1).
              Default: do not set.

  -timeout -- Timeout in seconds, 0 .. 25.5 s.
              Only valid in blocking, raw input mode (-raw=1 -ndelay=0).
              Default: do not set.

  -bufsize -- Buffer size for reading. Maximum length of read data.
              Default: 4096

  -errpref -- Prefix for error messages.
              Default: "IOSerial: "

Note that most options have no defaults: if such an option is not set
then the setting is left untouched. Some settings are not controlled
via options and done anyway.

Port setup:
 - Open device with O_RDWR | O_NOCTTY | O_NDELAY.
 - if "ndelay" option exists set non-blocking mode (FNDELAY flag).
 - Get current terminal options (tcgetattr).
 - Set CLOCAL | CREAD flags.
 - If "baud" option exists set baud rate (both input and output).
 - If "parity" option exists set parity.
 - If "raw" option exists set raw/canonical input
 - If "sfc" option exists enable/disable software flow control
 - Set raw output.
 - If "timeout" option exists then set timeout (with VMIN=0)
*/

#include <string>
#include "opt/opt.h"

class IOSerial {
  int fd; // file descriptor
  size_t bufsize;
  std::string errpref;
public:

  IOSerial(const Opt & opts);
  ~IOSerial();
  std::string read();
  void write(const std::string & msg);
};

#endif
