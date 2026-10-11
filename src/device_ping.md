### device_ping -- connect to a device bypassing the server

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
device_ping serial_jds6600 -dev "/dev/$1" -timeout 0.1 -- ":r01=." |
  grep -q "^:r01=2366400068\."
```
When communicating with the correct device the script reads and checks device
serial number and returns with zero status. The udev rule is:
```
ACTION=="add", SUBSYSTEM=="tty",\
   ATTRS{idVendor}=="1a86", ATTRS{idProduct}=="7523",\
   PROGRAM+="/etc/udev/ping_jds6600 %k",\
   GROUP="users", MODE="0660", SYMLINK+="jds6600"
```

