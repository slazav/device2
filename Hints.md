# Configuration hints


## Remote use

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

## Examples of udev rules

Here I have a few examples of udev rules for devices which I'm using. I
prefer to configure a unique symlink for each device using its serial
number, and to give access to users group.

A useful command to ead device attributes:
```
udevadm info -ap /sys/bus/usb/devices/1-2.3:1.1.
```

#### Keysight/Agilent/HP devices connected via USB (usbtmc driver)

Here I use exact serial number (to have a few devices):

```
ACTION=="add", KERNEL=="usbtmc*", ATTRS{serial}=="MY43005021",\
        GROUP="users", MODE="0660", SYMLINK+="gen1"
```

#### Pico oscilloscopes

A few devices can be connected, program should choose correct serial number.

```
ACTION=="add", SUBSYSTEM=="usb", ATTR{idVendor}=="0ce9", ATTR{idProduct}=="1009",\
   GROUP="users", MODE="0660"
```

#### Pico ADC

```
ACTION=="add", SUBSYSTEM=="usb", ATTR{idVendor}=="0ce9", ATTR{idProduct}=="1003",\
   GROUP="users", MODE="0660"
```

#### Korad/Velleman/Tenma 72-2550 power supply

Unfortunately devices do not have any ID/Serial.
Identification is done by devpath and depends on USB ports/hubs used!
Note: D-link USB hubs internally have 2 4-port hubs
 devpaths of connected devices are: 1.1, 1.2, 1.3, 1.4.1, 1.4.2, ...

```
ACTION=="add",\
  ATTRS{idVendor}=="0416", ATTRS{idProduct}=="5011", ATTRS{devpath}=="1.1",\
  GROUP="users", SYMLINK+="pst1"
```

#### ET4502 LCR meter

For using a few devices one can specify serial number by adding
`ATTRS{serial}=="48F7545B3737"`

```
ACTION=="add", SUBSYSTEMS=="usb",\
  ATTRS{idVendor}=="0483", ATTRS{idProduct}=="5740",\
  GROUP="users", SYMLINK+="lcr_meter"
```

