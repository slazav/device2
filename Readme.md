## Device server
---

This is a server for accessing devices and programs in complicated
experimental setups. Previously I used a tcl library (
https://github.com/slazav/tcl-device ), put that approach had a few
limitations.

All operations are done using "devices". They can be physical devices
connected via some interface, or programs (which can use other
devices if needed).

The server has a configuration file "/etc/device_server.txt" which lists
all avalible devices. For example, line "generator gpib -board 0 -address
6" says that communication with device "generator" is done using driver
gpib with parameters -board 0 -address 6.

The server does not know anything about commands used by certain device,
it just provides connection. For the next layer see DeviceRole library.

TODO:

Server provides IO locking (one device can be used by a few programs or
by severel threads in one program without collisions) and optional
high-level locking (one program can grab a device for a long time).

Library provides logging of all device communications: if there is a file
`/var/log/device_server/<name>` then all communication with the device
<name> is appended to this file. This allows to start/stop logging
without restarting and modifing programs.


### HTTP server

Users communicate with the server using GET requests of HTTP protocol.
URLs with three-components are used: `<device>`, `<command>`,
`<argument>`. For example, a request with "url" "`generator/cmd/FREQ?`"
sends command `FREQ?` to the device `generator` and returns answer.



### Configuration file

Configuration file contains one line per device. Empty lines and
comments (starting with `#`) are allowed. A few lines can be joined by
adding symbol `\` before end of line. Words can be quoted by single
or double quotes. Special symbols (`#`, `\`, quotes) can be typed by
adding `\` in front of them.

Each non-empty line should have the form:
```
<device name> <driver name> [<paramter> ...]
```

Parameters are pairs of words, first word in each pair should have prefix
`-`. Example: `-key1 val1 -key2 -val2`

Parameters are driver-specific.

### Commands

### Driver test -- a dummy driver for tests

...

