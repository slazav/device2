### TCL library

A TCL library Device2 for working with the server.

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
