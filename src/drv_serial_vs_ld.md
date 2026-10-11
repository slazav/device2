## Driver `serial_vs_ld` -- Agilent VS leak detector

Leak detector should be connected with null-modem cable.

### Parameters

* `-dev <str>`     -- Serial device filename (e.g. /dev/ttyUSB0). Required.

* `-timeout <v>`   -- Read timeout, seconds [0 .. 25.5]. Default: 5.0

* `-errpref <v>`   -- Prefix for error messages. Default "Agilent VS: "

* `-idn <v>`       -- Override output of *idn? command. Default: "Agilent VS leak detector"

