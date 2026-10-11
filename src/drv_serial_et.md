## Driver `serial_et` -- EastTester devices

Tested with E4502 LCR-meter.

### Parameters

* `-dev <v>`      -- Serial device filename (e.g. /dev/ttyACM0). Required.

* `-timeout <v>`  -- Read timeout, seconds [0 .. 25.5]. Default: 2.0

* `-errpref <v>` -- Prefix for error messages. Default "EastTester: "

* `-idn <v>`     -- Override output of *idn? command. Default: do not override.

Same as
```
serial -speed 9600 -parity 8N1 -cread 1 clocal 1\
  -ndelay 1 -sfc 1 -raw 1 -delay 0.1\
  -add_str \n -trim_str "\n\n" -read_cond always
  -timeout 2.0 -errpref "EastTester: "
```
