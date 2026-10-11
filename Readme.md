# Device2
---

## Overview

This is my client-server system for accessing devices and programs in an
experimental lab.

There is a server which works with "devices". They can be physical devices
connected in various ways, or programs with special interface. Each device
has a unique name, which is listed in the server configuration file.
Server knows how to open a device, send it a message and receive answer.
Clients can connect to the server using HTTP protocol and send messages to
devices.

This architecture has many advantages comparing to direct connections
between clients and devices:

* Clients know nothing about device connection details, they use only
device name to access it. All low-level connection parameters (e.g.
serial port configuration) are managed by the server.

* Multiple clients can access a single device without collisions. All
needed lockings are done by the server.

* A device opens on demand, when the first client start using it, and close
when all users disconnect.

* Clients can monitor all messages for any device.

* Remote access to devices can be easily implemented by either making a
tunnel for HTTP connections (there is no authentification in the server,
use `ssh` tunnels for this), or by attaching the remote client program
to the local server as a device.

I want to mention E.Sergeicheva and D.Zmeev who worked with this system
in Helsinki and Lancaster. They helped and motivated me a lot.

## System components

* [device_d](src/device_d.md) -- server program.

* [device_c](src/device_c.md) -- command-line client program.

* [device_ping](src/device_ping.md) -- a tool for connecting to a device bypassing the server

* [Basic TCL library](tcl/tcl.md)

* [SPP protocol...](SPP.md)

* [Confiduration hints...](Hints.md)

### Available drivers:

Some drivers are very general, they just send messages and receive
answers without any knowledge of specific devices. They work for most
SCPI-capable machinery connected via serial port, GPIB, LXI, VXI11,
usbtmc, for command-line programs connected via SPP protocol.

Other drivers keep information about connection details for specific
devices. For example, I have a bunch of `serial_*` drivers for various
leak detectors which require non-trivial serial port configuration.

Some drivers keep even deeper information about device. They may
introduce high-level text commands to replace binary communication (see
`i2c_*` drivers), set correct delays etc.

The general idea is to keep the communication as low-level as possible
here. I have a higher-level library [device-role](repo:slazav/device2)
for knowlege about devices. But exact balance is not clear.


* [test] -- A dummy driver for tests. Repeats incoming messages. No parameters.

* [spp](src/drv_spp.md) -- Communicating with programs via "Simple Pipe protocol"

* [usbtmc](src/drv_usbtmc.md) -- Using usbtmc kernel module (Agilent/Keysight USB devices, etc.)

* [serial](src/drv_serial.md) -- RS232 connections with full control of the serial port.
  Base for `serial_*` drivers.

  * [serial_asm340](src/drv_serial_asm340.md) -- Pfeiffer Adixen ASM340 leak detector.

  * [serial_l300](src/drv_serial_l300.md) -- Phoenix/Leybold L300i leak detector.

  * [serial_vs_ld](src/drv_serial_vs_ld.md) -- Agilent VS leak detector.

  * [serial_tenma_ps](src/drv_serial_tenma_ps.md) -- Korad/Velleman/Tenma power supplies.

  * [serial_et](src/drv_serial_et.md) -- EastTester devices (e.g. E4502 LCR-meter).

  * [serial_hm310t](src/drv_serial_hm310t.md) -- Hanmatec HM310T power supply.

  * [serial_jds6600](src/drv_serial_jds6600.md) -- Junctek JDS6600 signal generator.

  * [serial_simple](src/drv_serial_simple.md) -- Serial driver with reasonable default settings.
    Works with old Agilent/HP and Keithley devices.

* [net](src/drv_net.md) -- Network devices, LXI.

  * [net_gpib_prologix](src/drv_net_gpib_prologix.md) -- Prologix gpib2eth converter.

* [gpib](src/drv_gpib.md) -- GPIB devices via linux-gpib library.

* [vxi](src/drv_vxi.md) -- VXI-11 network devices.

* [i2c](src/drv_i2c.md) -- Communication with linux i2c bus via text commands. Base for `i2c_*` drivers

  * [i2c_bmp280](src/drv_i2c_bmp280.md) -- BMP280 pressure/temperature sensor via i2c

  * [i2c_ssd1306](src/drv_i2c_ssd1306.md) -- SSD1306 i2c display controller via i2c (128x64, 128x32 OLED displays)

---
V.Zavjalov, 2020-2026, vl.zavjalov at gmail dot com
