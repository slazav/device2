## Driver `drv_serial_hm310t` -- Hanmatec HM310T power supply

### Parameters

* `-dev <v>`      -- Serial device filename (e.g. /dev/ttyACM0). Required.

* `-timeout <v>`  -- Read timeout, seconds [0 .. 25.5]. Default: 5.0

* `-errpref <v>` -- Prefix for error messages. Default "HM310T: "

* `-idn <v>`     -- Output of *idn? command. Default: "HM310T"

### Commands introduced by the driver

The device uses binary modbus protocol for communication. Here I introduced an
SCPI-like command set on top of it.

*  *idn? -- get ID string (set artificially in the driver)
*  out? -- get output state, 0 or 1
*  stat:raw? -- protection status mask (raw data)
*  stat? -- protection status in human-readable form (could be incomplete)
*  spec:raw? -- "specification and type", no idea what is it, for my device it is 
*  tail:raw? -- "tail classification", no idea what is it, , for my device it is 
*  dpt:raw? -- return decimal point positions as raw data
*  dpt? -- return decimal point positions for volts, amps, watts (should be "2 3 3")
*  volt:meas? -- return measured voltage [V]
*  curr:meas? -- return measured current [A]
*  pwr:meas?  -- return measured power [W] (does not work?)
*  volt? -- return voltage set value [V]
*  curr? -- return current set value [A]
*  ovp?  -- get over voltage protection [V]
*  ocp?  -- get over current protection [A]
*  opp?  -- get over power protection [W] (doesn not work?)
*  addr? -- get modbus slave address (should be 1)

*  out [0|1] -- set output state
*  volt <volts> -- set voltage
*  curr <amps> -- set current
*  ovp <volts> -- set over voltage protection
*  ocp <amps> -- set over current protection
*  opp <watts> -- set over power protection

Letter case is ignored. Command for setting modbus address is skipped,
there is no need for it in the USB device.

*  :r<num> -- read Modbus register

### Problems

* power reading and power protection do not work in my device
* can't switch OVP and OCP mode remotely - no documented registers
* can clear OCP state remotely (by changing current and switching output off), but not OVP
* "specification and type", "tail classification" registers -- no understanding
* protection status mask -- doesn not match the documentation. At least OVP and OCP flags work for me

### Modbus registers

00 0         -- ?
01 0     r/w -- output on/off
02 2     r   -- protect status
03 3210  r   -- specification and type (?)
04 19267 r   -- tail classification (?)
05 563   r   -- decimal point positions
16 0     r   -- voltage display value
17 0     r   -- current display value
18 0     r   -- power display value H
19 0     r   -- power display value L
20 3200
21 10200
22 4
23 64256
32 3840  r/w -- OVP value
33 10000 r/w -- OCP value
34 1     r/w -- OPP value H
35 34464 r/w -- OPP value L
48 3000  r/w -- set voltage
49 1000  r/w -- set current
64 0
65 1
66 0
67 0
68 0
39321 1  r/w -- communication address

