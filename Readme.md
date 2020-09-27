## Device2
---

This is a client-server system for accessing devices and programs in
experimental setups. Previous version was a tcl library (
https://github.com/slazav/tcl-device ), that that approach had a few
limitations.

Server works with "devices". They can be physical devices or programs
with special interface (they can use other devices if needed).

Devices are listed in the configuration file `/etc/device2/devices.cfg`.
For example, line `generator gpib -board 0 -address 6` says that
communication with device `generator` is done using driver `gpib` with
parameters `-board 0 -address 6`.


### Server

Clients communicate with the server using GET requests of HTTP protocol.
URLs with three-components are used: `<device>`, `<command>`,
`<argument>`. For example, a request to
`http://<server>:<port>/generator/ask/FREQ?`" sends phrase `FREQ?` to the
device `generator` and returns answer. Commands are driver-specific,
aruments are device-specific. Most drivers support `ask` command to write
message to the devace an get answer.

On success a response with code 200 and answer of the device in the
message body is returned. On error a response with code 400 is returned.
Error description is written in `Error` header and in the message body.

The server does not know what it sends to a device and what answer is
expected, it just provides connection. For the next layer see DeviceRole
library. It defines certain "roles" for devices with common high-level
commands (e.g. "generator" role has a "set frequency" command).

### Locking

Server and device drivers provide IO locking (one device can be used by a
several programs or threads without collisions)

TODO: and optional high-level locking (one program can grab a device for
a long time).

## TODO

- grab/release commands for locking a device
- open/close/list/help commands
- logging commands
- authorisation, https

### Device configuration file

Configuration file contains one line per device. Empty lines and
comments (starting with `#`) are allowed. A few lines can be joined by
adding symbol `\` before end of line. Words can be quoted by single
or double quotes. Special symbols (`#`, `\`, quotes) can be typed by
adding `\` in front of them. Case of characters is important.

Each non-empty, non-comment line should have the form:
```
<device name> <driver name> [<parameter> ...]
```

Parameters are pairs of words, first word in each pair should have prefix
`-`. Example: `-key1 val1 -key2 -val2`

Parameters are driver-specific.

### Special device SERVER

Device with name SERVER is a special device for controlling the device server
itself. It can not be redefined in the configuration file.

Supported actions:

* `log_level/<N>` -- Set logging level of the server (0 - no messages, 1
- server messages,  2 - connections, 3 - communications with devices) and
return new value.

* `log_level` -- Return current logging level.

* `devices` or `list` -- Show list of all known devices.

* `use/<device>` -- Use the device in this connection. Argument is device name.
Usually a device is opened (if it is not opened yet) on demand, then some
action is requested. This action can be used to open and check the device
before sending any command to it (e.g. to process errors separately).

* `release/<device>` -- Notify the server that this device is not going
to be used by this connection. Device is closed if no other connections use it.
Argument is device name. Normally devices are closed when session is ended
and no other sessions are using the device. This command can be used to
close the device without closing the session. If some other session uses
the device it will stay opened.

* `usleep/<time>` -- Sleep for some time. Number of microseconds is taken
from the argument and returned in the answer.

* `repeat/<arg>` -- Return the argument in the answer.


### Driver `test` -- a dummy driver for tests

Supported configuration options: none

Supported actions:
* ask -- just repeat the message

### Driver `spp` -- a "Simple Pipe protocol".

This driver implements "Simple pipe prococol" for comminicating with
programs using stdin/stdout unix pipes.
Used in a few of my projects (pico_rec, graphene),
described in https://github.com/slazav/tcl-device (see Readme.md)

Supported configuration options:
* -prog -- name of the program
* -open_timeout -- timeout for device opening, seconds (default 20.0).
* -read_timeout -- timeout for reading from the device (default  5.0).

Supported actions:
* ask -- send the command and read answer.


