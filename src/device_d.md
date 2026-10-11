## device_d -- server program for device2 system

### HTTP communication

Clients communicate with the server using GET requests of HTTP protocol.
URLs with up to three components are used: `<action>/<device>/<message>`.
For example, a request to `http://<server>:<port>/ask/generator/FREQ?`" sends phrase
`FREQ?` to the device `generator` and returns answer.

On success a response with code 200 and answer of the device in the
message body is returned. On error a response with code 400 is returned.
Error description is written in `Error` header and in the message body.

The server does not know what it sends to a device and what answer is
expected, it just provides connection (For the next layer see 
DeviceRole library, https://github.com/slazav/tcl-device_role).

Supported actions:

* `ask/<device>/<message>` -- Send message to a device, return answer.

* `devices` or `list` -- Show list of all known devices.

* `info/<device>` -- Print information about a device.

* `reload` -- Reload device configuration. If case of errors in the file
old configuration is kept.

* `close/<device>` -- Close device. It will be reopened if needed.

* `ping` -- Check connection to the server. Returns nothing.

* `get_time` -- Get system time (unix seconds with microsecond precision)

* `use/<device>` -- Use a device in this connection. Usually a device is
open (if it is not open yet) on demand, then `ask` action is called.
This action can be used to open and check the device before sending any
message to it (e.g. to process open errors separately).

* `release/<device>` -- Notify the server that this device is not going
to be used by this connection anymore. Device is closed if no other
connection uses it. Normally devices are closed
when session is ended and no other sessions are using the device.

* `log_start/<device>` -- Any user can see all communications of every device.
To do it one should start with `log_start` action. The buffer of size
1024 lines is created for this connection, all messages send to the
device, all answers and errors received will be written to the buffer
(with "<<", ">>", and "EE" prefixes). If the buffer already exists it will
be cleared.

* `log_finish/<device>` -- Stop logging, delete the log buffer for this
connection.

* `log_get/<device>` -- Get contents of the log buffer, clear it. If logging
is stopped return error.

* `lock/<device>` -- Lock the device for single use. Normally no locking
is needed, many clients can communicate with the device without collisions.
But in some cases one may want to lock the device to prevent others from
using it. This is done by the `lock` action, only if
the device has no other users.

* `unlock/<device>` -- Unlock the device previously locked with the `lock` command.

* `set_conn_name/<name>` -- Set connection name. Each connection to the
server has a unique number and unique text name (by default it is
"#<conn_number>"). Client can change the name using this function. Name
set by user can not start with "#". If name is empty, the default value
"#<conn_number>" is set. If name already belongs to another connection error
occures. This feature can be used to detect another instances of the same
program.

* `get_conn_name` -- Get connection name.

* `list_conn_names` -- List all connections.

* `release_all` -- Release (and unlock) all devices, reset connection name
to default value.

There are two problems with locks and unique connection names (and
generally with session handling):

* If server is restarted, connections are reopened and clients do not
know about it. Locks and connection names will be lost in this situation.
It is recommended to update locks and names regularly, e.g. at the
beginning of each measurement cycle.

* Server can keep the connection when the client program already finished.
Locks and connection name are still there, and can affect the client if
it will  restart connection (e.g if the program is restarted). It is
recommended to run `release_all` action to unlocked devices and reset
connection name to default value before disconnecting.

### Starting the server

Usage:
* `device_d <options>`

Options:

* `-C, --cfgfile <arg>` -- Server configuration file (default: `/etc/device/device_d.cfg`).
* `-D, --devfile <arg>` -- Device list file (default: `/etc/device/devices.cfg`).
* `-a, --addr <arg>`    -- IP address to listen. Use "*" to listen everywhere (default: `127.0.0.1`).
* `-p, --port <arg>`    -- TCP port for connections (default: `8082`).
* `-f, --dofork`        -- Do fork and run as a daemon.
* `-S, --stop`          -- Stop running daemon (found by pid-file).
* `-R, --reload`        -- Reload configuration of running daemon (found by pid-file).
* `-U, --user <arg>`    -- Run as user (default: empty, do not switch user).
* `-v, --verbose <arg>` -- Verbosity level (default: 1):
  - 0 - write nothing;
  - 1 - write some information on server start/stop;
  - 2 - write about opening/closing connections and devices;
  - 3 - write all messages sent to devices and received from them.
* `-l, --logfile <arg>` -- Log file, "-" for stdout.
  (default: `/var/log/device_d.log` in daemon mode, "-" in console mode.
* `-P, --pidfile <arg>` -- Pid file (default: `/var/run/device_d.pid`)
* `--test`              -- Test mode with connection number limited to 1.
* `-h, --help`          -- Print help message and exit.
* `--pod`               -- Print help message in POD format and exit.


### Server configuration file

The default configuration file is `/etc/device/device_d.cfg`. It
can be used to override default values for some of the command-line options.
Following parameters can be set there: `addr`, `port`, `logfile`,
`pidfile`, `devfile`, `user`, `verbose`

The file contains one line per parameter. Empty lines and comments (starting
with `#`) are allowed. A few lines can be joined by adding symbol `\`
before end of line. Words can be quoted by single or double quotes.
Special symbols (`#`, `\`, quotes) can be typed by adding `\` in front of
them. Case of characters is important. Each non-empty, non-comment line
should have the form:
```
<parameter name> <parameter value>
```

### Device configuration file

Default location of the device configuration file is
`/etc/device2/devices.cfg` It can be changed with server command-line
option `-D` or in the server configuration file.

The file contains one line per device. Empty lines and comments (starting
with `#`) are allowed. A few lines can be joined by adding symbol `\`
before end of line. Words can be quoted by single or double quotes.
Special symbols (`#`, `\`, quotes) can be typed by adding `\` in front of
them. Case of characters is important. Each non-empty, non-comment line
should have the form:
```
<device name> <driver name> [-<parameter> <value> ...]
```

Device name should be non-empty and should not contain ` `, `\n`, `\t`,
`\` and `/` characters.

Parameters are driver-specific, see documentation for each driver.
List of drivers can be found in (../Readme.md).

If the file contains errors, server prints error message in the log and
keep old configuration (if any). If after starting the server you see no
devices in the `list` action output, try to do `reload` and see error
message.


### Signal handling

Server exits on SIGTERM, SIGINT, SIGQUIT signals. Device
list is re-read on SIGHUP signal. If `device_d` program is called with
`--stop`/`--reload` parameter it will send SIGTERM/SIGHUP to a running
server (found by pid-file). Reloading can be also done by any client
with `reload` action.

