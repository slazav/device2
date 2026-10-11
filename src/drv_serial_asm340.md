##* Driver `serial_asm340` -- Pfeiffer Adixen ASM340 leak detector

Leak detector should be in Advanced I/O mode and connected with
null-modem cable.

### Parameters

* `-dev <v>`       -- Serial device filename (e.g. /dev/ttyUSB0)
                      Required.

* `-timeout <v>`   -- Read timeout, seconds [0 .. 25.5]
                      Default: 5.0

* `-sfc (0|1)`     -- Software flow control (both 0 and 1 should work).
                      Default: 1

* `-errpref <str>` -- Prefix for error messages.
                      Default "ASM340: "

* `-idn <v>`       -- Override output of *idn? command.
                      Default: "Adixen ASM340 leak detector"

Same as
```
serial -speed 9600 -parity 8N1 -cread 1 clocal 1\
  -vmin 0 -ndelay 0 -icrnl 0 -raw 1 -sfc 1 -delay 0\
  -opost 1 -lcase 1 -ack_str "\x06" -nack_str "\x15"\
  -add_str "\r" -trim_str "\r" -read_cond always\
  -timeout 5.0 -errpref "ASM340: " -idn "Adixen ASM340 leak detector"
```
