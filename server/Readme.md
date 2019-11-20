## dev_server code

`dev_server.cpp` -- main function; command line options; daemonization; pid-file control.

`http_server.{cpp,h}` -- libmicrohttpd-related stuff; Run HTTP server, transfer requests to DevManager.

`dev_manager.{cpp,h}` -- device manager: open/close devices, process commands.

`drivers.{cpp,h}` -- device drivers.

`locks.h` -- inter-thread locks.

`mhd_locks.h` -- locking interface from libmicrohttpd library. pthread/win32 locks.

## Locks

There is a lock system (see lock.h) where you can obtain a lock for a
named resource. It is used in a few places:

The lock `"manager"` is used in the device manager to protect
modifications of the `devices` structure. At the moment it is done only
when reading configuration and only in the device manager constructor.
Thus no locking is actually needed. The plan is to introduce a command
for re-reading configuration when the server is running.

Locks `"manager:<name>"` are used in each device to protect modifications
of the `users` structure. This is done when a new connection sends a
command to the device (user is added) or then such a connection
disappeares (user removed).

Each device driver has methods `open, close, cmd` which may be called
from different threads. Also driver can use some extermal resource which
may need locking. It is responsibility of the driver to create locks.
It is recommended to use name `"device:<name>"` for device-specific locks
(as when two clients sends a command to the same device), name `"driver:<name>"`
for driver specific locks, `"resource:<address>"` for resource locks
(as when two drivers/devices access a single physical device).

