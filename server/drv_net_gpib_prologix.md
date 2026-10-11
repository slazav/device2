##  Driver `net_gpib_prologix` -- Prologix gpib2eth converter.

I was using this device long time ago. Here I adopted the old code
without testing!

### Parameters

* `-addr <v>`      -- Network address or IP.
                      Required.
* `-port <v>`      -- Port number.
                      Default: "5025" (lxi raw protocol).
* `-timeout <v>`   -- Read timeout, seconds. No timeout if <=0.
                      Default 5.0.
* `-bufsize <v>`   -- Buffer size for reading. Maximum length of read data.
                      Default: 4096
* `-errpref <v>`   -- Prefix for error messages.
                      Default: "IOSerial: "
* `-idn <v>`       -- Override output of *idn? command.
                      Default: empty string, do not override.
* `-gpib_addr <v>` -- GPIB address of the target device

