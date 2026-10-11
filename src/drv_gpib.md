## Driver `gpib` -- GPIB devices via linux-gpib library

By default reads answer from the device only if there is a question mark '?'
in the message.

### Parameters

* `-addr <N>`      -- GPIB address.
                      Required.

* `-board <N>`     -- GPIB board number.
                      Default: 0.

* `-timeout <str>` -- I/O timeout: none, 10us, 30us, 100us ... 300s, 1000s
                      Default 10s.

* `-open_timeout <str>` -- Open timeout, same values as for -read_timeout.
                      Default 3s.

* `-eot (1|0)`     -- Enable/disable assertion of EOI line with last data byte
                      Default: do not change

* `-eos_mode <v>`  -- Set end-of-string flags, any combination of R X B
                      characters for REOS, XEOS, BIN flags.
                      Only works with -eos option. Default: empty

* `-eos (0..255)`  -- Set end-of-string character.
                      Default: do not change

* `-secondary (1|0)` -- Set secondary GPIB address.

* `-bufsize <N>`   -- Buffer size for reading. Maximum length of read data.
                      Default: 4096

* `-errpref <str>` -- Prefix for error messages.
                      Default: "gpib: "

* `-idn <str>`     -- Override output of *idn? command.
                      Default: empty string, do not override.

* `-read_cond <v>` -- When do we need to read answer from a command:
                      always, never, qmark (if there is a question mark in the message),
                      qmark1w (question mark in the first word). Default: qmark1w.

* `-add_str <v>`   -- Add string to each message sent to the device.
                      Default: "\n"

* `-trim_str <v>`  -- Remove string from the end of received messages.
                      Default: "\n"

* `-delay <v>`     -- Delay after write command, s.
                      Default: 0
