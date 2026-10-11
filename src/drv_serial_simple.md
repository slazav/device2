## Driver ``serial_simple`  -- Serial driver with reasonable default settings

For example, it should work with old Agilent/HP and Keithley devices.

### Parameters

* `-dev <name>`  -- Serial device filename (e.g. /dev/ttyUSB0). Required.

* `-timeout <v>` -- Read timeout, seconds [0 .. 25.5]. Default: 5.0

* `-sfc (0|1)`   -- Software flow control. Default: 1

* `-errpref <v>` -- Prefix for error messages. Default "serial: ".

* `-idn <v>`     -- Override output of *idn? command. Default: do not override.

* `-read_cond <v>` -- When do we need to read answer from a command:
                      always, never, qmark (if there is a question mark in the message),
                      qmark1w (question mark in the first word). Default: qmark1w.
