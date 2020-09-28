## Device2
---

### Overview

This is a client-server system for accessing devices and programs in
experimental setups. Previous version was a tcl library (
https://github.com/slazav/tcl-device ), that approach had a few
limitations.

Server works with "devices". They can be physical devices or programs
with special interface. Devices have unique names. They are listed in the
configuration file `/etc/device2/devices.cfg`. For example, line
`generator gpib -board 0 -address 6` says that communication with device
`generator` is done using driver `gpib` with parameters `-board 0
-address 6`. Drivers know how to access devices (send messages and
recieve answers).

Clients can access devices via the server, using HTTP protocol. Client
can keep a connection to use single device without closing/opening it.
Each device is opened when the first user access it and closed when its
last user closes connection.

### Server

Clients communicate with the server using GET requests of HTTP protocol.
URLs with up to three components are used:
`<action>/<device>/<message>`. For example, a request to
`http://<server>:<port>/ask/generator/FREQ?`" sends phrase `FREQ?` to the
device `generator` and returns answer.

TODO: ban `/` character in the device name, or transfer messages via
URL arguments.

On success a response with code 200 and answer of the device in the
message body is returned. On error a response with code 400 is returned.
Error description is written in `Error` header and in the message body.

The server does not know what it sends to a device and what answer is
expected, it just provides connection. For the next layer see DeviceRole
library. It defines certain "roles" for devices with common high-level
commands (e.g. "generator" role has a "set frequency" command).

Supported actions:

* `devices` or `list` -- Show list of all known devices.

* `info/<device>` -- Print information about device.

* `ask/<device>/<message>` -- Send message to the device, return answer.

* `use/<device>` -- Use the device in this connection. Argument is device name.
Usually a device is opened (if it is not opened yet) on demand, then `ask`
action is called. This action can be used to open and check the device
before sending any message to it (e.g. to process errors separately).

* `release/<device>` -- Notify the server that this device is not going
to be used by this connection anymore. Device is closed if no other
connections use it. Argument is device name. Normally devices are closed
when session is ended and no other sessions are using the device.

* `lock/<device>` -- Lock the device for single use. Normally no locking
is needed, many clients can communicate with the device without collisions.
But in some cases one may want to lock the device to prevent others from
using it. This is done by the `lock` action, only if
the device has no other users.

* `unlock/<device>` -- Unlock the device previously locked with the `lock`
command. Devices are unlocked when the connection is closed, but
the server keeps the connection longer then it's needed (until new
connection appears). This can cause a problem if you lock the device,
reopen the connection, and try to use the same device. Thus it is
recommended to unlocked device when you finish with it.
Command returns errors if device is locked by somebody else, or not locked.

* `log_start/<device>` -- Any user can see all communications of every device.
To do it one should start with `log_start` action. The buffer of size 1024 lines
is created for this connection, all messages send to the device and received
from it will be written to the buffer (with "<<" and ">>" prefixes).
If the buffer already exist it will be cleared.

* `log_finish/<device>` -- Stop logging, delete the log buffer for this
connection.

* `log_get/<device>` -- Get contents of the log buffer, clear it. If logging
is not started return error.

* `ping` -- Check connection to the server. Returns nothing.

* `get_time` -- Get sysem time (unix seconds with microsecond precision)


### Device configuration file

Default location of the device configuration file is
`/etc/device2/devices.cfg` It can be changed by with server command-line
option `-D`.

The file contains one line per device. Empty lines and comments (starting
with `#`) are allowed. A few lines can be joined by adding symbol `\`
before end of line. Words can be quoted by single or double quotes.
Special symbols (`#`, `\`, quotes) can be typed by adding `\` in front of
them. Case of characters is important. Each non-empty, non-comment line
should have the form:
```
<device name> <driver name> [-<parameter> <value> ...]
```

Parameters are pairs of words, first word in each pair should have prefix
`-`. Example: `-key1 val1 -key2 -val2`

Parameters are driver-specific.

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
* -read_timeout -- timeout for reading from the device, seconds (default  5.0).

Supported actions:
* ask -- send the command and read answer.

### Running the server

Usage:
* `device_d <options>`

Options:

* `-C, --config <arg>`  -- Device configuration file (default: `/etc/device/devices.cfg`).
* `-p, --port <arg>`    -- TCP port for connections (default: `8082`).
* `-f, --dofork`        -- Do fork and run as a daemon.
* `-S, --stop`          -- Stop running daemon (found by pid-file).
* `-v, --verbose <arg>` -- Verbosity level (default: 1):
  - 0 - write nothing;
  - 1 - write some information on server start/stop;
  - 2 - write about opening/closing connections and devices;
  - 3 - write all messages sent to devices and recieved from them.
* `-l, --logfile <arg>` -- Log file, "-" for stdout.
  (default: `/var/log/device_d.log` in daemon mode, "-" in console mode.
* `-P, --pidfile <arg>` -- Pid file (default: `/var/run/device_d.pid`)
* `-h, --help`          -- Print help message and exit.
* `--pod`               -- Print help message in POD format and exit.

---
### Client

Program `device_c` is a command-line client for working with the server.

Usage:
* `device_c [<options>] ask <dev> <msg>` -- send message to the device, print answer
* `device_c [<options>] use_dev <dev>`   -- SPP interface to a device
* `device_c [<options>] use_srv`         -- SPP interface to the server
* `device_c [<options>] (list|devices)`  -- print list of available devices
* `device_c [<options>] info <dev>`      -- print information about device
* `device_c [<options>] ping`            -- check if the server is working
* `device_c [<options>] get_time`        -- get server system time

Options:
* `-s, --server <arg>` -- Server (default: http://localhost:8082).
* `-l, --lock`         -- Lock the device (only for `use_dev` action).
* `-h, --help`         -- Print help message and exit.
* `--pod`              -- Print help message in POD format and exit.

---
### Examples

Get device list. There are two devices available:
```
$ device_c list
graphene
test
```

Get information about a device. The device uses `spp` driver which runs program `graphene -i`
and talks with it using SPP interface. It is not used by anybody (and thus closed).
```
$ device_c info graphene
Device: graphene
Driver: spp
Driver arguments:
  -prog: graphene -i
Device is closed
Number of users: 0
```

Send message to a device and get answer. In this case we send `get_time` command to
the `graphene` program:
```
$ device_c ask graphene get_time
1601282446.214037
```

The server also has `get_time` action to get system time on the server:
```
$ device_c get_time
1601284005.581721
```

Use a device for multiple queries, without reopening it. In this case SPP protocol
is used: each message is followed by `#OK` or `#Error <message>` line.
Every input line is sent to the device as-is, answer is returned.

```
$ device_c use_dev graphene
#SPP001
Server: http://localhost:8082
Device: graphene
#OK
bad_command
#Error: Unknown command: bad_command
get_time 0
#Error: too many parameters
get_time
1601284282.200903
#OK
```

Use the server for multiple queries. In this mode you have access to all
server actions and can work with multiple devices. Every input line is
split into three words: `action`, `device`, and `message`. The
`read_word` library ( https://github.com/slazav/mapsoft2-libs/tree/master/read_words ) is used
to enable quotes, escape characters, multiline input etc. For example, if
you want to use spaces in the message, quote it.

```
$ device_c use_srv
#SPP001
Server: http://localhost:8082
#OK
ask graphene get_time
1601284389.596303
#OK
list
graphene
test

#OK
info graphene
Device: graphene
Driver: spp
Driver arguments:
  -prog: graphene -i
Device is open
Number of users: 1
You are currently using the device

#OK
```

---
### TODO

- locking named resource by a client
- logging commands
- authorisation, https
