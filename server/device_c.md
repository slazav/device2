## device_c -- client program for device2 system

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
