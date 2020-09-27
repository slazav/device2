## dev_server code

`device_s.cpp` -- main program; command line options; daemonization; pid-file control.

`http_server.{cpp,h}` -- libmicrohttpd-related stuff; Run HTTP server, transfer requests to DevManager.

`dev_manager.{cpp,h}` -- device manager: open/close devices, process commands.

`drivers.{cpp,h}` -- device drivers.

`device.{cpp,h}` -- A device object represents a device in
the configuration file.


## Definitions

* `connection` -- HTTP client can keep a connection to the
server and use single device without closing/opening it.
Each device stores list of user connections. It is opened
when the first user send calls the device and and closed
when the last connection where it was used disappeares.

* `device` -- Devices are external objects (physical devices,
over programs) which are accessed by the device server. The server does
not know how device commands but provides communication via drivers.
Devices, their drivers and driver options are listed in the configuration
file.

* `driver` -- An object which knows how to access a device.
Each device contains its own driver object.



