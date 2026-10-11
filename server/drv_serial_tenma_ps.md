### Driver `serial_tenma_ps` -- Korad/Velleman/Tenma power supplies

### Parameters

* `-dev <v>`      -- Serial device filename (e.g. /dev/ttyACM0). Required.

* `-timeout <v>`  -- Read timeout, seconds [0 .. 25.5]. Default: 5.0

* `-errpref <v>` -- Prefix for error messages. Default "TenmaPS: "

* `-idn <v>`     -- Override output of *idn? command. Default: do not override.

### Serial port configuration

Same as
```
serial -speed 9600 -parity 8N1 -cread 1 clocal 1\
  -vmin 0 -ndelay 0 -sfc 0 -raw 1 -delay 0.1\
  -opost 1 -olcuc 1 -read_cond qmark\
  -timeout 5.0 -errpref "TenmaPS: "
```

Note important settings:

- `-sfc 0`: Software flow control should be off. This allows receiving
zero status byte (if OVC, OVP is off and output is off). `-raw 1` is also
needed to pass special characters.

- `-delay 0.1`: It should be a delay between sending command to the
device and reading answer.
