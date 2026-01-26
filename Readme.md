# Device2
---

## Overview

This is a client-server system for accessing devices and programs in
experimental setups. Previous version was a tcl library (
https://github.com/slazav/tcl-device ) with a few annoying limitations.

There is a server which works with "devices". They can be physical devices
connected in various ways or programs with special interface. Each device
has a unique name, which is listed in the server configuration file.
Server knows how to open a device, send it a message and receive answer.

Clients can connect to the server using HTTP protocol and send messages to
devices. This architecture has many advantages comparing to direct connections
between clients and devices:

* Clients know nothing about device connection details, they use only
device name to access it. All low-level connection parameters (e.g.
serial port configuration) are managed by the server.

* Multiple clients can access a single device without collisions. All
needed lockings are done by the server.

* A device opens on demand, when the first client start using it, and close
when all users disconnect. Client should keep a permanent connection
to the server if it wants to use a device without closing/opening it.

* Any client can monitor all messages for any device.

* Remote access to devices can be easily implemented by either making a
tunnel for HTTP connections (there is no authentification in the server,
use `ssh` for this), or by attaching the remote client program in
`use_dev` mode, `ssh -T device_c use_dev <name>` to the local server as
a device.

The server support devices connected via linux-gpib and usbtmc kernel
drivers; serial and network devices (including VXI11 and raw LXI);
programs with SPP interface.

## Server

### HTTP communication with the server

Clients communicate with the server using GET requests of HTTP protocol.
URLs with up to three components are used: `<action>/<device>/<message>`.
(Note that symbol `/` is not allowed in device names). For example, a
request to `http://<server>:<port>/ask/generator/FREQ?`" sends phrase
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

* `unlock/<device>` -- Unlock the device previously locked with the `lock`
command.

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

Configuration file: Server configuration file can be used to override
default values for some of the command-line options. Following parameters
can be set in the configuration file: `addr`, `port`, `logfile`,
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

Signal handling: server exits on SIGTERM, SIGINT, SIGQUIT signals. Device
list is re-read on SIGHUP signal. If `device_d` program is called with
`--stop`/`--reload` parameter it will send SIGTERM/SIGHUP to a running
server (found by pid-file). Reloading can be also done by any client
with `reload` action.

### Device list file

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

Parameters are driver-specific.

If the file contains errors server prints error message in the log and
keep old configuration (if any). If after starting the server you see no
devices in the `list` action output, try to do `reload` and see error
message.


### Available drivers:

* `test` -- a dummy driver for tests.

* `spp` -- a "Simple Pipe protocol", talking with programs. Works.

* `usbtmc` -- for USB devises connect via usbtmc kernel module
(Agilent/Kesight devices). Works.

* `serial` -- RS232 connections with full control of the serial
port. Base for other serial_* drivers.

* `serial_asm340` -- Driver for Pfeiffer Adixen ASM340 leak detector
connected via RS232 (null-modem cable). Leak detector should be in
Advanced I/O mode. Works.

* `serial_l300` -- Driver for Phoenix/Leybold L300i leak detector
connected via RS232 (normal cable, not null-modem). Leak detector
settings (in Menu/Settings/Interfaces): Control locations: ALL or
RS232; RS232 settings: 19200 LF -- 8N1 -- ASCII. Works.

* `serial_vs_ld` -- Driver for Agilent VS leak detector.
Leak detector should be connected with null-modem cable.

* `serial_tenma_ps` -- Driver for Korad/Velleman/Tenma power supplies.
Works.

* `serial_et` -- Driver for EastTester devices.
Works with E4502 LCR meter.

* `serial_hm310t` -- Driver for Hanmatec HM310T power supply.

* `serial_jds6600` -- Driver for Junctek JDS6600 signal generator.

* `serial_simple` -- Serial driver with reasonable default settings.
Works with old Agilent/HP/Keythley devices.

* `net` -- Network connections. By default it works with LXI devices.

* `net_gpib_prologix` -- Driver for Prologix gpib2eth converter.
Not tested.

* `gpib` -- for devices connected via linux-gpib library.
Works.

* `vxi` -- for network devices connected via vxi-11 protocol.
Works, but have not been tested much.


### Driver `test` -- a dummy driver for tests

Just repeats any message sent to it.

Parameters: none


### Driver `spp` -- programs following "Simple Pipe protocol"

This driver implements "Simple pipe protocol" for communicating with
programs using stdin/stdout unix pipes (see the protocol description at
the end of this text) It is used in a few of my projects (pico_rec,
graphene), and in `device_c` client program.

Parameters:

* `-prog`          -- Program name. Required.

* `-open_timeout`  -- Timeout for opening, seconds. Default 20.0.

* `-read_timeout`  -- Timeout for reading, seconds. Default: 10.0.

* `-close_timeout`  -- Timeout for closing (before sending SIGTERM), seconds. Default: 5.0.

* `-errpref` -- Prefix for error messages. Default "spp: ".

* `-idn`     -- Override output of *idn? command. Default: do not override.


### Driver `usbtmc` -- USB devices using usbtmc kernel module

This driver supports devices connected via usbtmc kernel driver. It
should work with all usual Agilent/Keysight devices connected via USB.
Read timeout is set internally in the driver (5s by default?) but can be
modified by -timeout parameter.

By default the driver reads answer from the device only if there is a
question mark '?' in the first word of the message.

Parameters:

* `-dev <v>`      -- Serial device filename (e.g. /dev/usbtmc0)
                     Required.

* `-timeout <v>`  -- Timeout for reading, seconds
                     Default: 5.0

*  `-delay <v>`   -- Delay after write command, s.
                     Default: 0.01

* `-errpref <v>`  -- Prefix for error messages.
                     Default "usbtmc: ".

* `-idn <v>`      -- Override output of *idn? command.
                     Default: do not override.

* `-read_cond <v>` -- When do we need to read answer from a command:
                     `always`, `never`, `qmark` (if there is a question mark in
                     the message), qmark1w (question mark in the first word).
                     Default: `qmark1w`.

* `-add_str <v>`  -- Add string to each message sent to the device.
                     Default: "\n"

* `-trim_str <v>` -- Remove string from the end of received messages.
                     Default: "\n"


### Driver "gpib" -- GPIB devices using linux-gpib library

By default the driver reads answer from the device only if there is a
question mark '?' in the first word of the message.

Parameters:

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
                      `always`, `never`, `qmark` (if there is a question mark in
                      the message), qmark1w (question mark in the first word).
                      Default: `qmark1w`.

* `-add_str <v>`   -- Add string to each message sent to the device.
                      Default: "\n"

* `-trim_str <v>`  -- Remove string from the end of received messages.
                      Default: "\n"

* `-delay <v>`     -- Delay after write command, s.
                      Default: 0

### Driver "vxi" -- network devices via vxi-11 protocol

By default the driver reads answer from the device only if there is a
question mark '?' in the first word of the message.

Parameters:

* `-addr <N>`      -- IP/hostname.
                      Required.

* `-name <N>`      -- Instrument name ("instr0", "gpib0,10", etc.).
                      Required.

* `-rpc_timeout <float>`  -- I/O timeout in seconds.
                             Default: 2.0

* `-io_timeout <float>`   -- I/O timeout in seconds.
                             Default: 5.0

* `-lock_timeout <float>` -- I/O timeout in seconds.
                             Default: 2.0

* `-errpref <str>` -- Prefix for error messages.
                      Default: "vxi: "

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

###  Driver `net` -- network devices

By default the driver reads answer from the device only if there is a
question mark '?' in the first word of the message.

Defaults parameters correspond to LXI raw protocol.

Parameters:

* `-addr`           -- Network address or IP.
                       Required.
* `-port <N>`       -- Port number.
                       Default: "5025" (lxi raw protocol).
* `-timeout <N>`    -- Read timeout, seconds. No timeout if <=0.
                       Default 5.0.
* `-delay <v>`      -- Delay after write command, s.

* `-open_delay <v>` -- Delay before opening a connection, s. Default: 0.0.
                       This could be useful for some strange devices (Siglent power supplies)
                       which need some time between closing previous connection and opening
                       a new one.
                       Default: 0
* `-bufsize <N>`    -- Buffer size for reading. Maximum length of read data.
                       Default: 4096
* `-errpref <str>`  -- Prefix for error messages.
                       Default: "Driver_net: "
* `-idn <str>`      -- Override output of *idn? command.
                       Default: empty string, do not override.
* `-read_cond <v>`  -- When do we need to read answer from a command:
                       `always`, `never`, `qmark` (if there is a question mark in
                      the message), qmark1w (question mark in the first word).
                       Default: `qmark1w`.
* `-add_str <v>`    -- Add string to each message sent to the device.
                       Default: "\n"
* `-trim_str <v>`   -- Remove string from the end of received messages.
                       Default: "\n"

### Driver `net_gpib_prologix` -- devices connected via Prologix gpib2eth converter

Not tested!

Parameters:

* `-addr <v>`      -- Network address or IP.
                      Required.
* `-port <v>`      -- Port number.
                      Default: "1234".
* `-timeout <v>`   -- Read timeout, seconds. No timeout if <=0.
                      Default 5.0.
* `-bufsize <v>`   -- Buffer size for reading. Maximum length of read data.
                      Default: 4096
* `-errpref <v>`   -- Prefix for error messages.
                      Default: "Driver_net: "
* `-idn <v>`       -- Override output of *idn? command.
                      Default: empty string, do not override.
* `-addr <v>`      -- GPIB address

### Driver `serial` -- Serial devices

This is a very general driver with lots of parameters.
See: `Serial Programming Guide for POSIX Operating Systems
(https://www.cmrr.umn.edu/~strupp/serial.html)`, `stty(1)`, `tcsetattr(3)`.

Other serial drivers are based on it. As an example, configuration of
Pfeiffer ASM340 leak detector can be written as
```
asm340a  serial \
  -dev /dev/ttyUSB0 -speed 9600 -parity 8N1 -cread 1 -clocal 1\
  -timeout 2.0 -vmin 0 -ndelay 0 -icrnl 0\
  -sfc 1 -raw 1 -errpref "ASM340: " -delay 0\
  -ack_str \x06 -nack_str \x15 -add_str \r -trim_str \r
```

but there is a special driver which set all needed parameters:
```
asm340b  serial_asm340 -dev /dev/ttyUSB0
```

Note that most options have no defaults: if such an option is not set
then the linux serial port setting is left untouched.

#### Parameters: serial port setup.

* `-dev <name>`   -- Serial device filename (e.g. /dev/ttyUSB0)
                   Required.

* `-ndelay (1|0)` -- Non-blocking mode
                   Default: do not change.

* `-speed <N>`    -- Baud rate (both input and output), integer value.
                   POSIX values: 0,50,75,110,134,150,200,300,600,1200,
                     1800,2400,4800,9600,19200,38400.
                   Non-POSIX values: 57600, 115200, 230400, 460800,
                     500000, 576000, 921600, 1000000, 1152000, 1500000,
                     2000000, 2500000, 3000000, 3000000, 3500000, 4000000.
                   Default: do not change.

* `-ispeed <N>`   -- input boud rate
* `-ospeed <N>`   -- output boud rate

#### Parameters: stty control options

If option is missing no change is made.

* `-clocal (1|0)`  -- disable modem control signals
* `-cread (1|0)`   -- allow input to be received
* `-crtscts (1|0)` -- enable RTS/CTS handshaking
* `-cs <N>`        -- set character size to N bits, N in [5..8]
* `-cstopb (1|0)`  -- use two stop bits per character
* `-hup (1|0)`     -- send a hangup signal when the last process closes the tty
* `-parenb (1|0)`  -- generate parity bit in output and expect parity bit in input
* `-parodd (1|0)`  -- set odd parity (or even parity with '-')
* `-cmspar (1|0)`  -- use "stick" (mark/space) parity

#### Parameters: stty input settings

If option is missing no change is made.

* `-icrnl (1|0)`   -- translate carriage return to newline
* `-inlcr (1|0)`   -- translate newline to carriage return
* `-igncr (1|0)`   -- ignore carriage return
* `-iuclc (1|0)`   -- translate uppercase characters to lowercase
* `-iutf8 (1|0)`   -- assume input characters are UTF-8 encoded
* `-brkint (1|0)`  -- breaks cause an interrupt signal
* `-ignbrk (1|0)`  -- ignore break characters
* `-imaxbel (1|0)` -- beep and do not flush a full input buffer on a character
* `-inpck (1|0)`   -- enable input parity checking
* `-ignpar (1|0)`  -- ignore characters with parity errors
* `-istrip (1|0)`  -- clear high (8th) bit of input characters
* `-parmrk (1|0)`  -- mark parity errors (with a 255-0-character sequence)
* `-ixany (1|0)`   -- let any character restart output, not only start character
* `-ixoff (1|0)`   -- enable sending of start/stop characters
* `-ixon (1|0)`    -- enable XON/XOFF flow control

#### Parameters: stty output settins

If option is missing no change is made.

* `-bs <N>`        -- backspace delay style, N in [0..1] (no delay, 100ms)
* `-cr <N>`        -- carriage return delay style, N in [0..3] (no delay, column-dep, 100ms, 150ms)
* `-ff <N>`        -- form feed delay style, N in [0..1] (no delay, 2s)
* `-nl <N>`        -- newline delay style, N in [0..1] (no delay, 100ms)
* `-tab <N>`       -- horizontal tab delay style, N in [0..3] (no delay, column-dep, 100ms, tab to space)
* `-vt <N>`        -- vertical tab delay style, N in [0..1] (no delay, 2s)
* `-ocrnl (1|0)`   -- translate carriage return to newline
* `-onlcr (1|0)`   -- translate newline to carriage return-newline
* `-onlret (1|0)`  -- newline performs a carriage return
* `-onocr (1|0)`   -- do not print carriage returns in the first column
* `-ofdel (1|0)`   -- use delete characters for fill instead of NUL characters
* `-ofill (1|0)`   -- use fill (padding) characters instead of timing for delays
* `-olcuc (1|0)`   -- translate lowercase characters to uppercase
* `-opost (1|0)`   -- postprocess output

#### Parameters: stty local settings.

If option is missing no change is made.

* `-echo (1|0)`    -- echo input characters
* `-echoctl (1|0)` -- echo control characters in hat notation ('^c')
* `-echoe (1|0)`   -- echo erase characters as backspace-space-backspace
* `-echok (1|0)`   -- echo a newline after a kill character
* `-echoke (1|0)`  -- BS-SP-BS entire line on line kill
* `-echonl (1|0)`  -- echo newline even if not echoing other characters
* `-echoprt (1|0)` -- echo erased characters backward, between '\' and '/'
* `-extproc (1|0)` -- enable "LINEMODE"; useful with high latency links
* `-flusho (1|0)`  -- discard output
* `-icanon (1|0)`  -- enable special characters: erase, kill, werase, rprnt (canonical input)
* `-iexten (1|0)`  -- enable non-POSIX special characters
* `-isig (1|0)`    -- enable interrupt, quit, and suspend special characters
* `-noflsh (1|0)`  -- disable flushing after interrupt and quit special characters
* `-tostop (1|0)`  -- stop background jobs that try to write to the terminal
* `-xcase (1|0)`   -- with icanon, escape with '\' for uppercase characters (obsolete?)

#### Parameters: combination settings

Different from stty(1) options!
If option is missing no change is made.

* `-parity <V>`  -- Parity (8N1, 7N1, 7O1, 7S1).

* `-raw (1|0)`   -- Raw/canonical input.
                    canonical input: +icanon +echo +echoe +isig
                    raw input: -icanon -echo -echoe

* `-sfc (1|0)`   -- Enable/disable software flow control (ixon ixof ixany)

* `-nlcnv (1|0)` -- Enable/disable newline conversion NL->NL+CR, CR->NL (icrnl, onlcr)
                    opost should be enabled for onlcr?

* `-lcase (1|0)` -- Enable/disable upper to lower case conversion (iuclc, olcuc)
                    opost should be enabled for olcuc?

* `-timeout <v>` -- Timeout in seconds, [0..25.5] s.
                    Only valid in blocking, raw input mode (-raw=1 -ndelay=0).

* `-vmin <N>`    -- min number of characters [0..255]
                    Only valid in blocking, raw input mode (-raw=1 -ndelay=0).

#### Parameters: other

* `-delay <v>`     -- Delay after write command, s.
                      Default: 0.1

* `-errpref <v>`   -- Prefix for error messages.
                      Default: "serial: "

* `-idn <v>`       -- Override output of *idn? command.
                      Default: empty string, do not override.

* `-add_str <v>`   -- Add string to each message sent to the device.
                      Note: using terminal setting -onlcr you can convert NL to NL+CR
                      Default: empty string

* `-trim_str <v>`  -- Remove string from the end of received messages.
                      Default: empty string

* `-ack_str <v>`
* `-nack_str <v>`  -- Some devices send ack/nack sequences at the end of each message.
                      It can be ack/nack characters (0x9/0x15), or "ok"/"#?" strings.
                      if -ack option is set then reading is done until ack (or nack
                      if it is set too) sequence is met. The sequences are removed
                      from the output message; error is returned if nack received.
                      Default: empty strings.

* `-read_cond <v>` -- When do we need to read answer from a command:
                      always, never, qmark (there is a question mark in the message),
                      qmark1w (question mark in the first word). Default: always.

* `-flush_on_err (0|1)` -- Flush serial buffers if Input/output error happens.
                           Default: 1


### Driver `serial_simple` -- Serial driver with reasonable default settings.

Should work with old Agilent/HP devices.

Parameters:

* `-dev <name>`  -- Serial device filename (e.g. /dev/ttyUSB0).
                    Required.

* `-timeout <v>` -- Read timeout, seconds [0 .. 25.5].
                    Default: 5.0

* `-sfc (0|1)`   -- Software flow control.
                    Default: 1

* `-errpref <v>` -- Prefix for error messages. Default "serial: ".

* `-idn <v>`     -- Override output of *idn? command.
                    Default: do not override.

* `-read_cond <v>` -- When do we need to read answer from a command:
                      always, never, qmark (if there is a question mark in the message),
                      qmark1w (question mark in the first word). Default: qmark1w.

Same as
```
serial -speed 9600 -parity 8N1 -cread 1 clocal 1\
  -vmin 0 -ndelay 0 -icrnl 1 -raw 1 -sfc 1 -delay 0.1\
  -opost 0 -add_str "\n" -trim_str "\n" -read_cond qmark1w\
  -timeout 5.0"
```

### Driver `serial_asm340` -- Pfeiffer Adixen ASM340 leak detector

Leak detector should be in Advanced I/O mode and connected with
null-modem cable.

Parameters:

* `-dev <v>`       -- Serial device filename (e.g. /dev/ttyUSB0)
                      Required.

* `-timeout <v>`   -- Read timeout, seconds [0 .. 25.5]
                      Default: 5.0

* `-sfc (0|1)`     -- Software flow control (both 0 and 1 should work).
                      Default: 1

* `-errpref <str>` -- Prefix for error messages.
                      Default "ASM340: "

* `-idn <v>`       -- Override output of *idn? command.
                      Default: "Adixen ASM340 leak detector"

Same as
```
serial -speed 9600 -parity 8N1 -cread 1 clocal 1\
  -vmin 0 -ndelay 0 -icrnl 0 -raw 1 -sfc 1 -delay 0\
  -opost 1 -lcase 1 -ack_str "\x06" -nack_str "\x15"\
  -add_str "\r" -trim_str "\r" -read_cond always\
  -timeout 5.0 -errpref "ASM340: " -idn "Adixen ASM340 leak detector"
```

### Driver `serial_l300` -- Phoenix/Leybold L300i leak detector

Leak detector settings:
- Menu/Settings/Interfaces/Control locations:  ALL or RS232
- Menu/Settings/Interfaces/RS232:  19200 LF -- 8N1 -- ASCII

Cable: normal RS232, not null-modem.
Hardware handshake is disabled in the driver,
there is no need to use special cable to emulate it.

SCPI Command set is very similar to one of inficon Modul1000 leak
detector.

Parameters:

* `-dev <v>`       -- Serial device filename (e.g. /dev/ttyUSB0)
                      Required.

* `-timeout <v>`   -- Read timeout, seconds [0 .. 25.5]
                      Default: 5.0

* `-errpref <str>` -- Prefix for error messages.
                      Default "ASM340: "

* `-idn <v>`       -- Override output of *idn? command.
                      Default: "Adixen ASM340 leak detector"

Same as
```
serial -speed 19200 -parity "8N1"\
 -cread  1 -clocal 1 -icrnl 0 -raw 1 -sfc 1 -crtscts 0\
  -add_str "\r" -trim_str "\r" -read_cond always\
  -timeout 5.0 -errpref "L300: " -idn "PhoeniXL300"
```

### Driver `serial_vs_ld` -- Agilent VS leak detector

Leak detector should be connected with null-modem cable.

Parameters:

* `-dev <str>`     -- Serial device filename (e.g. /dev/ttyUSB0)
                      Required.

* `-timeout <v>`   -- Read timeout, seconds [0 .. 25.5]
                      Default: 5.0

* `-errpref <v>`   -- Prefix for error messages.
                      Default "Agilent VS: "

* `-idn <v>`       -- Override output of *idn? command.
                      Default: "Agilent VS leak detector"


### Driver `serial_tenma_ps` -- Korad/Velleman/Tenma power supplies

Parameters:

* `-dev <v>`      -- Serial device filename (e.g. /dev/ttyACM0)
                    Required.

* `-timeout <v>`  -- Read timeout, seconds [0 .. 25.5]
                    Default: 5.0

* `-errpref <v>` -- Prefix for error messages.
                    Default "TenmaPS: "

* `-idn <v>`     -- Override output of *idn? command.
                    Default: do not override.

Same as
```
serial -speed 9600 -parity 8N1 -cread 1 clocal 1\
  -vmin 0 -ndelay 0 -sfc 0 -raw 1 -delay 0.1\
  -opost 1 -olcuc 1 -read_cond qmark\
  -timeout 5.0 -errpref "TenmaPS: "
```

Note important setting:

- `-sfc 0`: Software flow control should be off. This allows receiving
zero status byte (if OVC, OVP is off and output is off). `-raw 1` is also
needed to pass special characters.

- `-delay 0.1`: It should be a delay between sending command to the
device and reading answer.

### Driver `serial_et` -- EastTester devices

Parameters:

* `-dev <v>`      -- Serial device filename (e.g. /dev/ttyACM0)
                    Required.

* `-timeout <v>`  -- Read timeout, seconds [0 .. 25.5]
                    Default: 2.0

* `-errpref <v>` -- Prefix for error messages.
                    Default "EastTester: "

* `-idn <v>`     -- Override output of *idn? command.
                    Default: do not override.

Same as
```
serial -speed 9600 -parity 8N1 -cread 1 clocal 1\
  -ndelay 1 -sfc 1 -raw 1 -delay 0.1\
  -add_str \n -trim_str "\n\n" -read_cond always
  -timeout 2.0 -errpref "EastTester: "
```

### Driver `drv_serial_hm310t` -- Hanmatec HM310T power supply

Parameters:

* `-dev <v>`      -- Serial device filename (e.g. /dev/ttyUSB0)
                    Required.

* `-timeout <v>`  -- Read timeout, seconds [0 .. 25.5]
                    Default: 5.0

* `-errpref <v>` -- Prefix for error messages.
                    Default "HM310T: "

* `-idn <v>`     -- Override output of *idn? command.
                    Default: "HM310T"

Command set:

The device uses binary modbus protocol for communication. Here we introduce an
SCPI-like command set on top of it.

*  *idn? -- get ID string (set artificially in the driver)
*  out? -- get output state, 0 or 1
*  stat:raw? -- protection status mask (raw data)
*  stat? -- protection status in human-readable form (could be incomplete)
*  spec:raw? -- "specification and type", no idea what is it, for my device it is.
*  tail:raw? -- "tail classification", no idea what is it, , for my device it is.
*  dpt:raw? -- return decimal point positions as raw data
*  dpt? -- return decimal point positions for volts, amps, watts (should be "2 3 3")
*  volt:meas? -- return measured voltage [V]
*  curr:meas? -- return measured current [A]
*  pwr:meas?  -- return measured power [W] (does not work?)
*  volt? -- return voltage set value [V]
*  curr? -- return current set value [A]
*  ovp?  -- get over voltage protection [V]
*  ocp?  -- get over current protection [A]
*  opp?  -- get over power protection [W] (doesn not work?)
*  addr? -- get modbus slave address (should be 1)

*  out [0|1] -- set output state
*  volt <volts> -- set voltage
*  curr <amps> -- set current
*  ovp <volts> -- set over voltage protection
*  ocp <amps> -- set over current protection
*  opp <watts> -- set over power protection

Letter case is ignored. Command for setting modbus address is skipped,
there is no need for it in the USB device.

*  :r<num> -- read Modbus register

Problems:

* power reading and power protection do not work in my device
* can't switch OVP and OCP mode remotely - no documented registers
* can clear OCP state remotely (by changing current and switching output off), but not OVP
* "specification and type", "tail classification" registers -- no understanding
* protection status mask -- doesn not match the documentation. At least OVP and OCP flags work for me

### Driver `serial_jds6600` -- Junctek JDS6600 signal generator

Parameters:

* `-dev <v>`      -- Serial device filename (e.g. /dev/ttyACM0)
                    Required.

* `-timeout <v>`  -- Read timeout, seconds [0 .. 25.5]
                    Default: 2.0

* `-errpref <v>` -- Prefix for error messages.
                    Default "JDS6600: "

* `-idn <v>`     -- Override output of *idn? command.
                    Default: JDS6600

Same as
```
serial -speed 115200 -parity 8N1 -cread 1 -clocal 1\
  -sfc 0 -onlcr 1 -igncr 1\
  -add_str '\n' -trim_str '\n' -read_cond always\
  -errpref "JDS6600: " -idn "JDS6600"
```

---
## Client

Program `device_c` is a command-line client for working with the server.

Usage:
* `device_c [<options>] ask <dev> <msg> ...` -- send message to the device, print answer
* `device_c [<options>] use_dev <dev>`   -- SPP interface to a device
* `device_c [<options>] use_srv`         -- SPP interface to the server
* `device_c [<options>] (list|devices)`  -- print list of available devices
* `device_c [<options>] info <dev>`      -- print information about device
* `device_c [<options>] reload`          -- reload device configuration
* `device_c [<options>] close`           -- close device (it will be reopened if needed)
* `device_c [<options>] monitor <dev>`   -- monitor all communication of the device
* `device_c [<options>] ping`            -- check if the server is working
* `device_c [<options>] get_time`        -- get server system time
* `device_c [<options>] get_srv`         -- get server address

Options:
* `-s, --server <arg>` -- Server (default: localhost).
* `-p, --port <arg>`   -- Port (default: 8082).
* `-v, --via <arg>`    -- Connect to the server through a tunnel.
                          Argument: name of the gateway.
* `--via_cmd <arg>`    -- Specify command template for making the tunnel, with $L, $R, $H,
                          and $G for local port, remote port, remote host and gateway.
                          Default: /usr/bin/ssh -f -L \"$L\":\"$H\":\"$R\" \"$G\" sleep 20
* `-l, --lock`         -- Lock the device (only for `use_dev` action).
* `-h, --help`         -- Print help message and exit.
* `--pod`              -- Print help message in POD format and exit.

Client configuration file (`/etc/device/device_c.cfg`) is similar to the
server one.  Following parameters can be set in the configuration file:
`server`, `port`, `via`, `via_cmd`.


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
the `graphene` device:
```
$ device_c ask graphene get_time
1601282446.214037
```

Send message to a device and get answer. Create a database with FLOAT values
using the `graphene` device:
```
$ device_c ask graphene create new_db FLOAT "my new database"
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
`read_words` library ( https://github.com/slazav/mapsoft2-libs/tree/master/read_words ) is used
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

### device_ping -- test a device

This is a simple program for talking to a device bypassing the server.

Usage: `device_ping <driver> [<driver options>] -- <msg>`,
where `<driver>` and `<driver options>` are same as in the
`devices.cfg` configuration file. Program opens connection with a device,
sends a message and prints answer.

Using this program one can write a test script for using in `udev` rules.
This can help with multiple devices which can not be distinguised by `udev`.

#### Example:

I have a few devices based on QinHeng Electronics CH340 serial converter.
They can not be distinguised by `udev`. One of devices is JDS6600 signal
generator which can be tested by script `/etc/udev/ping_jds6600`:
```
#!/bin/sh
device_ping \
  serial -dev "/dev/$1" -speed 115200 -parity 8N1 -sfc 0 -onlcr 1 -igncr 1\
  -add_str '\n' -trim_str '\n' -cread 1 -clocal 1 -timeout 0.1 -- ":r01=."
```
With a correct device name the script prints string `:r01=2366400068.` which
contains device serial number. The udev rule is:
```
ACTION=="add", SUBSYSTEM=="tty",\
   ATTRS{idVendor}=="1a86", ATTRS{idProduct}=="7523",\
   PROGRAM+="/etc/udev/ping_jds6600 %k", RESULT==":r01=2366400068.*"\
   GROUP="users", MODE="0660", SYMLINK+="jds6600"
```

### Remote use

There are two ways how to configure remote access to your devices. First,
you can connect to the remote server by HTTP protocol. Note that device
server does not have any security features, and it's not safe to allow
connections from outside. I recommend using an ssh tunnel to
the remote server. This can be done with -via parameter of `device_c`
command.

Other option is to use external devices connected to the local server.
The `device_c` program implements SPP protocol when using in `use_dev`
and `use_srv` modes. Thus it can be used as a device. If you have device
servers running on both computers and want to access remote device
`mydev`, just write in the local configuration something like this:
```
mydev   spp -prog "ssh -T comp2 device_c use_dev mydev"
```

This approach allows you to mix local and remote devices on your computer
and access them through the local server. Note that in this configuration
you should have timeouts of the spp driver larger then timeouts of the
device's driver. Currently default read/open timeouts are:
10s/20s for spp, 3s/10s for gpib, 5s for serial, usbtmc, and net drivers.

Do not connect device to itself as it will obviously crash the server.
```
mydev   spp -prog "device_c use_dev mydev"
```

### TCL library

There is a TCL library Device2 for working with the server.

"New" interface:

* `Device2:addr` -- variable with the server address. When library is loading
it is updated by running `device_c get_srv` and thus syncronized with
device_c configuration file (TODO: what about -via setting?).

* `Device2:get <action> <device> <msg> ...` -- the most general function
for communicating with the server. All extra arguments are joined with `<msg>`.

* `Device2:ask <dev> <msg>`
* `Device2:list`
* `Device2:reload`
* `Device2:close`
* `Device2:ping`
* `Device2:get_time`
* `Device2:info <dev>`
* `Device2:use <dev>`
* `Device2:release <dev>`
* `Device2:lock <dev>`
* `Device2:unlock <dev>`
* `Device2:log_start <dev>`
* `Device2:log_finish <dev>`
* `Device2:log_get <dev>` -- syntactic sugar for all actions

"Old" interface is almost compatable with Device library (https://github.com/slazav/tcl-device).
For each device `itcl` object with `cmd`, `lock`, `unlock` methods can be created. Methods
`read` and `write` are missing (they were not widely used and I want to get rid of them).
```
Device <name>
<name> cmd *IDN?
DeviceDelete <name>
```

For example, DeviceRole library can be switched to Device2 just by
replacing `package use Device` by `package use Device2`.

## Simple pipe protocol (SPP)
---

#### version 001

There are two programs, "server" and "client". Client runs the server
program and communicate with it using unix pipes. All data are read and
written line by line in a human-readable form if possible.

When connection is opened, server writes a line with the special symbol
('#' in this example, but it can be any symbol), protocol name and
version: `#SPP<version>`. Then it can write some text for the user which can be
ignored. Then it either writes `#Error: <message>` and exits or writes
`#OK` and start listening for user requests. Simbol '#' here is a special
symbol which was selected in the beginning of the conversation.

Request is one line of text.

Answer of the server is a few lines of text, followed by '#Error:
<message>' or '#OK' line. Lines of the answer text starting with the
'#' symbol should be protected by doubling the symbol.

It is recommended to implement *idn? command which returns ID of the
device.

#### version 002

If any fatal error appear server can print a line `#Fatal: <message>`
and exit.

* TODO: requests with several lines
* TODO: timeouts, safe closing of the channel...
* TODO: raw data transfer

#### Conversation example
```
$ ./spp_server_test.tcl
#SPP002
Welcome, dear user!
Please, type "help" if you do not know what to do.
#OK
h
#Error: Unknown command: h
help
spp_server_test -- an example of the command-line interface program.
Commands:
  write <k> <v> -- write value v into a memory slot k=0..9
  read <k>      -- read value from a memory slot k
  list -- list all commands
  help -- show this message

#OK
write 1 abc
#OK
read
#Error: wrong # args: should be "testSrv0 read k"
read 1
abc
#OK
```

---
V.Zavjalov, 2020-2024, vl.zavjalov at gmail dot com
