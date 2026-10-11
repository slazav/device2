## Driver `spp` -- Communicating with programs via "Simple Pipe protocol"

This driver implements "Simple pipe protocol" for communicating with
programs using stdin/stdout unix pipes. The protocol is described in
https://github.com/slazav/tcl-device (see Readme.md) It is used in a few
of my projects (pico_rec, graphene), and in `device_c` client program.

### Parameters

* `-prog`          -- Program name. Required.

* `-open_timeout`  -- Timeout for opening, seconds. Default 20.0.

* `-read_timeout`  -- Timeout for reading, seconds. Default: 10.0.

* `-close_timeout`  -- Timeout for closing (before sending SIGTERM), seconds. Default: 5.0.

* `-errpref` -- Prefix for error messages. Default "spp: ".

* `-idn`     -- Override output of *idn? command. Default: do not override.
