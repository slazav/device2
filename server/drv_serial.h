#ifndef DRV_SERIAL_H
#define DRV_SERIAL_H

/*************************************************/
/*
 * Access to a serial device

This is a very general driver with lots of parameters.
See: `Serial Programming Guide for POSIX Operating Systems
(https://www.cmrr.umn.edu/~strupp/serial.html)`, `stty(1)`, `tcsetattr(3)`.

Other serial drivers are based on it. As an example, configuration of
Pfeiffer ASM340 leak detector can be written as
```
asm340a  serial \
  -dev /dev/ttyUSB0 -speed 9600 -parity 8N1 -cread 1 -clocal 1\
  -timeout 2.0 -vmin 0 -ndelay 0 -icrnl 0\
  -sfc 1 -raw 1 -errpref "ASM340: " -delay 0\
  -ack_str \x06 -nack_str \x15 -add_str \r -trim_str \r
```

but there is a special driver which set all needed parameters:
```
asm340b  serial_asm340 -dev /dev/ttyUSB0
```

Note that most options have no defaults: if such an option is not set
then the linux serial port setting is left untouched.

#### Parameters: serial port setup.

* `-dev <name>`   -- Serial device filename (e.g. /dev/ttyUSB0)
                   Required.

* `-ndelay (1|0)` -- Non-blocking mode
                   Default: do not change.

* `-speed <N>`    -- Baud rate (both input and output), integer value.
                   POSIX values: 0,50,75,110,134,150,200,300,600,1200,
                     1800,2400,4800,9600,19200,38400.
                   Non-POSIX values: 57600, 115200, 230400, 460800,
                     500000, 576000, 921600, 1000000, 1152000, 1500000,
                     2000000, 2500000, 3000000, 3000000, 3500000, 4000000.
                   Default: do not change.

* `-ispeed <N>`   -- input boud rate
* `-ospeed <N>`   -- output boud rate

#### Parameters: stty control options

If option is missing no change is made.

* `-clocal (1|0)`  -- disable modem control signals
* `-cread (1|0)`   -- allow input to be received
* `-crtscts (1|0)` -- enable RTS/CTS handshaking
* `-cs <N>`        -- set character size to N bits, N in [5..8]
* `-cstopb (1|0)`  -- use two stop bits per character
* `-hup (1|0)`     -- send a hangup signal when the last process closes the tty
* `-parenb (1|0)`  -- generate parity bit in output and expect parity bit in input
* `-parodd (1|0)`  -- set odd parity (or even parity with '-')
* `-cmspar (1|0)`  -- use "stick" (mark/space) parity

#### Parameters: stty input settings

If option is missing no change is made.

* `-icrnl (1|0)`   -- translate carriage return to newline
* `-inlcr (1|0)`   -- translate newline to carriage return
* `-igncr (1|0)`   -- ignore carriage return
* `-iuclc (1|0)`   -- translate uppercase characters to lowercase
* `-iutf8 (1|0)`   -- assume input characters are UTF-8 encoded
* `-brkint (1|0)`  -- breaks cause an interrupt signal
* `-ignbrk (1|0)`  -- ignore break characters
* `-imaxbel (1|0)` -- beep and do not flush a full input buffer on a character
* `-inpck (1|0)`   -- enable input parity checking
* `-ignpar (1|0)`  -- ignore characters with parity errors
* `-istrip (1|0)`  -- clear high (8th) bit of input characters
* `-parmrk (1|0)`  -- mark parity errors (with a 255-0-character sequence)
* `-ixany (1|0)`   -- let any character restart output, not only start character
* `-ixoff (1|0)`   -- enable sending of start/stop characters
* `-ixon (1|0)`    -- enable XON/XOFF flow control

#### Parameters: stty output settins

If option is missing no change is made.

* `-bs <N>`        -- backspace delay style, N in [0..1] (no delay, 100ms)
* `-cr <N>`        -- carriage return delay style, N in [0..3] (no delay, column-dep, 100ms, 150ms)
* `-ff <N>`        -- form feed delay style, N in [0..1] (no delay, 2s)
* `-nl <N>`        -- newline delay style, N in [0..1] (no delay, 100ms)
* `-tab <N>`       -- horizontal tab delay style, N in [0..3] (no delay, column-dep, 100ms, tab to space)
* `-vt <N>`        -- vertical tab delay style, N in [0..1] (no delay, 2s)
* `-ocrnl (1|0)`   -- translate carriage return to newline
* `-onlcr (1|0)`   -- translate newline to carriage return-newline
* `-onlret (1|0)`  -- newline performs a carriage return
* `-onocr (1|0)`   -- do not print carriage returns in the first column
* `-ofdel (1|0)`   -- use delete characters for fill instead of NUL characters
* `-ofill (1|0)`   -- use fill (padding) characters instead of timing for delays
* `-olcuc (1|0)`   -- translate lowercase characters to uppercase
* `-opost (1|0)`   -- postprocess output

#### Parameters: stty local settings.

If option is missing no change is made.

* `-echo (1|0)`    -- echo input characters
* `-echoctl (1|0)` -- echo control characters in hat notation ('^c')
* `-echoe (1|0)`   -- echo erase characters as backspace-space-backspace
* `-echok (1|0)`   -- echo a newline after a kill character
* `-echoke (1|0)`  -- BS-SP-BS entire line on line kill
* `-echonl (1|0)`  -- echo newline even if not echoing other characters
* `-echoprt (1|0)` -- echo erased characters backward, between '\' and '/'
* `-extproc (1|0)` -- enable "LINEMODE"; useful with high latency links
* `-flusho (1|0)`  -- discard output
* `-icanon (1|0)`  -- enable special characters: erase, kill, werase, rprnt (canonical input)
* `-iexten (1|0)`  -- enable non-POSIX special characters
* `-isig (1|0)`    -- enable interrupt, quit, and suspend special characters
* `-noflsh (1|0)`  -- disable flushing after interrupt and quit special characters
* `-tostop (1|0)`  -- stop background jobs that try to write to the terminal
* `-xcase (1|0)`   -- with icanon, escape with '\' for uppercase characters (obsolete?)

#### Parameters: combination settings

Different from stty(1) options!
If option is missing no change is made.

* `-parity <V>`  -- Parity (8N1, 7N1, 7O1, 7S1).

* `-raw (1|0)`   -- Raw/canonical input.
                    canonical input: +icanon +echo +echoe +isig
                    raw input: -icanon -echo -echoe

* `-sfc (1|0)`   -- Enable/disable software flow control (ixon ixof ixany)

* `-nlcnv (1|0)` -- Enable/disable newline conversion NL->NL+CR, CR->NL (icrnl, onlcr)
                    opost should be enabled for onlcr?

* `-lcase (1|0)` -- Enable/disable upper to lower case conversion (iuclc, olcuc)
                    opost should be enabled for olcuc?

* `-timeout <v>` -- Timeout in seconds, [0..25.5] s.
                    Only valid in blocking, raw input mode (-raw=1 -ndelay=0).

* `-vmin <N>`    -- min number of characters [0..255]
                    Only valid in blocking, raw input mode (-raw=1 -ndelay=0).

#### Parameters: other

* `-delay <v>`     -- Delay after write command, s.
                      Default: 0.1

* `-errpref <v>`   -- Prefix for error messages.
                      Default: "serial: "

* `-idn <v>`       -- Override output of *idn? command.
                      Default: empty string, do not override.

* `-add_str <v>`   -- Add string to each message sent to the device.
                      Note: using terminal setting -onlcr you can convert NL to NL+CR
                      Default: empty string

* `-trim_str <v>`  -- Remove string from the end of received messages.
                      Default: empty string

* `-ack_str <v>`
* `-nack_str <v>`  -- Some devices send ack/nack sequences at the end of each message.
                      It can be ack/nack characters (0x9/0x15), or "ok"/"#?" strings.
                      if -ack option is set then reading is done until ack (or nack
                      if it is set too) sequence is met. The sequences are removed
                      from the output message; error is returned if nack received.
                      Default: empty strings.

* `-read_cond <v>` -- When do we need to read answer from a command:
                      always, never, qmark (there is a question mark in the message),
                      qmark1w (question mark in the first word). Default: always.

* `-flush_on_err (0|1)` -- Flush serial buffers if Input/output error happens.
                           Default: 1

*/

#include <memory>
#include "drv.h"
#include "drv_utils.h"
#include "opt/opt.h"
#include <string>
#include "opt/opt.h"

class Driver_serial: public Driver {
  int fd; // file descriptor
  std::string errpref, idn;
  std::string ack,nack,add,trim;
  read_cond_t read_cond;
  double delay;
  bool flush_on_err;

public:

  Driver_serial(const Opt & opts);
  ~Driver_serial();

  std::string read() override;
  void write(const std::string & msg) override;
  std::string ask(const std::string & msg) override;
};

#endif
