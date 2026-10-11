## Driver `serial_l300` -- Phoenix/Leybold L300i leak detector

### Leak detector settings

- Menu/Settings/Interfaces/Control locations:  ALL (or RS232)
-  Menu/Settings/Interfaces/RS232:  19200 LF -- 8N1 -- ASCII

Cable: normal RS232, not null-modem. Hardware handshake is disabled in
the driver, there is no need to use special cable to emulate it.

SCPI command set is very similar to one of inficon Modul1000 leak detector.

### Parameters

* `-dev <v>`       -- Serial device filename (e.g. /dev/ttyUSB0)
                      Required.

* `-timeout <v>`   -- Read timeout, seconds [0 .. 25.5]
                      Default: 5.0

* `-errpref <str>` -- Prefix for error messages.
                      Default "ASM340: "

* `-idn <v>`       -- Override output of *idn? command.
                      Default: "Adixen ASM340 leak detector"
*/

### Serial port settings

Same as
```
serial -speed 19200 -parity "8N1"\
 -cread  1 -clocal 1 -icrnl 0 -raw 1 -sfc 1 -crtscts 0\
  -add_str "\r" -trim_str "\r" -read_cond always\
  -timeout 5.0 -errpref "L300: " -idn "PhoeniXL300"
```
