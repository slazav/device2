#ifndef DRV_SERIAL_HM310T_H
#define DRV_SERIAL_HM310T_H

/*************************************************/
/*
 * Driver `drv_serial_hm310t` -- Hanmatec HM310T power supply
 *

Parameters:

* `-dev <v>`      -- Serial device filename (e.g. /dev/ttyACM0)
                    Required.

* `-timeout <v>`  -- Read timeout, seconds [0 .. 25.5]
                    Default: 5.0

* `-errpref <v>` -- Prefix for error messages.
                    Default "HM310T: "

* `-idn <v>`     -- Override output of *idn? command.
                    Default: "HM310T"

Command set:

The device uses binary modbus protocol for communication. Here we introduce an
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

Problems:

* power reading and power protection do not work in my device
* can't switch OVP and OCP mode remotely - no documented registers
* can clear OCP state remotely (by changing current and switching output off), but not OVP
* "specification and type", "tail classification" registers -- no understanding
* protection status mask -- doesn not match the documentation. At least OVP and OCP flags work for me

Modbus registers:

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


*/

#include "drv_serial.h"

class Driver_serial_hm310t: public Driver_serial {

  // Decimal precisions - we get then from the device in constuctor
  int Ve, Ie, Pe;
  int dev_addr; // Modbus slave address. Always use 1 for the USB device.

  Opt add_opts(const Opt & opts){
    opts.check_unknown({"dev", "timeout", "errpref", "idn"});
    Opt o(opts);
    o.put("speed",  9600);  // baud rate
    o.put("parity", "8N1"); // character size, parity, stop bit
    o.put("cread",  1);     // always set cread=1
    o.put("clocal", 1);     // always set clocal=1
    o.put("vmin",   0); // should be set with timeout
    o.put("ndelay", 0); // should be set with timeout
    o.put("sfc",    0); // allow zero bytes
    o.put("raw",    1); // raw mode!
    o.put("delay",0.01); // 10ms delay after write
    o.put("opost",  0); // no output postprocessing
    o.put("read_cond",  "always"); // alays read answers

    // set defaults (only it no values are set by user)
    o.put_missing("timeout", 5.0);
    o.put_missing("errpref", "HM310T: ");
    o.put_missing("idn", "HM310T");

    return o;
  }

  // modbus crc calculation
  uint16_t modbus_crc(const uint8_t *buf, size_t len );

  // read word from given address (Modbus RTU, function 3)
  uint16_t modbus_func3(const uint16_t addr);
  // write value to a given address (Modbus RTU, function 6)
  void modbus_func6(const uint16_t addr, const uint16_t val);

  // get decimap precisions from the device
  void read_dpt();

  // Helpers for reading and converting voltage, current, power to V,A,W units
  // (using decimal precisions Ve, Ie, Pe)
  std::string read_volt(const uint16_t addr);
  std::string read_curr(const uint16_t addr);
  std::string read_pwr(const uint16_t addrH, const uint16_t addrL);
  void write_volt(const uint16_t addr, const std::string & arg);
  void write_curr(const uint16_t addr, const std::string & arg);
  void write_pwr(const uint16_t addrH, const uint16_t addrL, const std::string & arg);

public:
  Driver_serial_hm310t(const Opt & opts): Driver_serial(add_opts(opts)), dev_addr(1){
    read_dpt(); // update decimal point positions from the device
  }

  std::string read() override {
    throw Err() << "serial_hm310t driver: read() is not supported, use ask()"; }
  void write(const std::string & msg) override;
  std::string ask(const std::string & msg) override;
};

#endif
