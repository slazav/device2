## code

`device_d.cpp` -- server program; command line options; daemonization; pid-file control.

`http_server.{cpp,h}` -- libmicrohttpd-related stuff; Run HTTP server, transfer requests to DevManager.

`dev_manager.{cpp,h}` -- device manager: open/close devices, process commands.

`drivers.{cpp,h}` -- device drivers.

`device.{cpp,h}` -- A device object represents a device in
the configuration file.

`device_c.cpp` -- client program
