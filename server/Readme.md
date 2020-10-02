## code

Server side:

`device_d.cpp` -- server program; command line options; daemonization; pid-file control.

`http_server.{cpp,h}` -- libmicrohttpd-related stuff; Run HTTP server, transfer requests to DevManager.

`dev_manager.{cpp,h}` -- device manager: open/close devices, process commands.

`device.{cpp,h}` -- A device object represents a device in
the configuration file.

`drv_*{cpp,h}` -- device drivers.

`tmc.h` -- header file for usbtmc kernel driver.

Client side:

`device_c.cpp` -- client program

`tun.{cpp,h}` -- utlilities for making ssh tunnel.

