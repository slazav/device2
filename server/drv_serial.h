#ifndef DRV_SERIAL_H
#define DRV_SERIAL_H

/*************************************************/
/*
 * Access to a serial device
 *
 * See:
 *  https://www.cmrr.umn.edu/~strupp/serial.html
 *  stty(1), tcsetattr(3)

Options:
  -dev <name>   -- Serial device filename (e.g. /dev/ttyUSB0)
                   Required.

  -ndelay (1|0) -- Non-blocking mode
                   Default: do not change.

  -speed <N>    -- Baud rate (both input and output), integer value.
                   Posix values: 0,50,75,110,134,150,200,300,600,1200,
                     1800,2400,4800,9600,19200,38400.
                   Non-posix falues: 57600, 115200, 230400, 460800,
                     500000, 576000, 921600, 1000000, 1152000, 1500000,
                     2000000, 2500000, 3000000, 3000000, 3500000, 4000000.
                   Default: do not change.
  -ispeed <N>   -- input boud rate
  -ospeed <N>   -- output boud rate

  Control options. If option is missing no change is made.

  -clocal (1|0)  -- disable modem control signals
  -cread (1|0)   -- allow input to be received
  -crtscts (1|0) -- enable RTS/CTS handshaking
  -cs <N>        -- set character size to N bits, N in [5..8]
  -cstopb (1|0)  -- use two stop bits per character
  -hup (1|0)     -- send a hangup signal when the last process closes the tty
  -parenb (1|0)  -- generate parity bit in output and expect parity bit in input
  -parodd (1|0)  -- set odd parity (or even parity with '-')
  -cmspar (1|0)  -- use "stick" (mark/space) parity

  Input settings. If option is missing no change is made.

  -icrnl (1|0)   -- translate carriage return to newline
  -inlcr (1|0)   -- translate newline to carriage return
  -igncr (1|0)   -- ignore carriage return
  -iuclc (1|0)   -- translate uppercase characters to lowercase
  -iutf8 (1|0)   -- assume input characters are UTF-8 encoded
  -brkint (1|0)  -- breaks cause an interrupt signal
  -ignbrk (1|0)  -- ignore break characters
  -imaxbel (1|0) -- beep and do not flush a full input buffer on a character
  -inpck (1|0)   -- enable input parity checking
  -ignpar (1|0)  -- ignore characters with parity errors
  -istrip (1|0)  -- clear high (8th) bit of input characters
  -parmrk (1|0)  -- mark parity errors (with a 255-0-character sequence)
  -ixany (1|0)   -- let any character restart output, not only start character
  -ixoff (1|0)   -- enable sending of start/stop characters
  -ixon (1|0)    -- enable XON/XOFF flow control

  Output settins. If option is missing no change is made:

  -bs <N>        -- backspace delay style, N in [0..1] (no delay, 100ms)
  -cr <N>        -- carriage return delay style, N in [0..3] (no delay, column-dep, 100ms, 150ms)
  -ff <N>        -- form feed delay style, N in [0..1] (no delay, 2s)
  -nl <N>        -- newline delay style, N in [0..1] (no delay, 100ms)
  -tab <N>       -- horizontal tab delay style, N in [0..3] (no delay, column-dep, 100ms, tab to space)
  -vt <N>        -- vertical tab delay style, N in [0..1] (no delay, 2s)
  -ocrnl (1|0)   -- translate carriage return to newline
  -onlcr (1|0)   -- translate newline to carriage return-newline
  -onlret (1|0)  -- newline performs a carriage return
  -onocr (1|0)   -- do not print carriage returns in the first column
  -ofdel (1|0)   -- use delete characters for fill instead of NUL characters
  -ofill (1|0)   -- use fill (padding) characters instead of timing for delays
  -olcuc (1|0)   -- translate lowercase characters to uppercase
  -opost (1|0)   -- postprocess output

  Local settings. If option is missing no change is made.

  -echo (1|0)    -- echo input characters
  -echoctl (1|0) -- echo control characters in hat notation ('^c')
  -echoe (1|0)   -- echo erase characters as backspace-space-backspace
  -echok (1|0)   -- echo a newline after a kill character
  -echoke (1|0)  -- BS-SP-BS entire line on line kill
  -echonl (1|0)  -- echo newline even if not echoing other characters
  -echoprt (1|0) -- echo erased characters backward, between '\' and '/'
  -extproc (1|0) -- enable "LINEMODE"; useful with high latency links
  -flusho (1|0)  -- discard output
  -icanon (1|0)  -- enable special characters: erase, kill, werase, rprnt (canonical input)
  -iexten (1|0)  -- enable non-POSIX special characters
  -isig (1|0)    -- enable interrupt, quit, and suspend special characters
  -noflsh (1|0)  -- disable flushing after interrupt and quit special characters
  -tostop (1|0)  -- stop background jobs that try to write to the terminal
  -xcase (1|0)   -- with icanon, escape with '\' for uppercase characters (obsolete?)

  Combination settings. Different from stty(1) options!

  -parity <V>  -- Parity (8N1, 7N1, 7O1, 7S1).
                  Default: do not change.

  -raw (1|0)   -- Raw/canonical input.
                  canonical input: +icanon +echo +echoe +isig
                  raw input: -icanon -echo -echoe
                  Default: do not change.

  -sfc (1|0)   -- Enable/disable software flow control (ixon ixof ixany)
                  Default: do not change.

  -nlcnv (1|0) -- Enable/disable newline conversion (icrnl, onlcr)
                  opost should be enabled for onlcr?

  -lcase (1|0) -- Enable/disable upper to lower case conversion (iuclc, olcuc)
                  opost should be enabled for olcuc?

  -timeout     -- Timeout in seconds, [0..25.5] s.
                  Only valid in blocking, raw input mode (-raw=1 -ndelay=0).
                  Default: do not change.

  -vmin <N>    -- min number of characters [0..255]
                  Only valid in blocking, raw input mode (-raw=1 -ndelay=0).

  -errpref     -- Prefix for error messages.
                 Default: "IOSerial: "

Note that most options have no defaults: if such an option is not set
then the setting is left untouched. Some settings are not controlled
via options and done anyway.

*/

#include <memory>
#include "drv.h"
#include "opt/opt.h"
#include <string>
#include "opt/opt.h"

class Driver_serial: public Driver {
  int fd; // file descriptor
  std::string errpref;

public:

  Driver_serial(const Opt & opts);
  ~Driver_serial();
  std::string read();
  void write(const std::string & msg);
  std::string ask(const std::string & msg) override;
};

#endif
