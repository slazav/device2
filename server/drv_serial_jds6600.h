#ifndef DRV_SERIAL_JDS6600_H
#define DRV_SERIAL_JDS6600_H

/*************************************************/
/*
 * Driver `serial_jds6600` -- Junctek JDS6600 signal generator
 *

Options:

* `-dev <v>`      -- Serial device filename (e.g. /dev/ttyACM0)
                    Required.

* `-timeout <v>`  -- Read timeout, seconds [0 .. 25.5]
                    Default: 5.0

* `-errpref <v>` -- Prefix for error messages.
                    Default "JDS6600: "

* `-idn <v>`     -- Override output of *idn? command.
                    Default: JDS6600

Device interfce is some variant of Modbus with text messages:
":<command><register>=<comma-separated integer arguments>."
where command is w,r,i,b, register is 0..99

Registers (for my device)
:r00=40.         -- 40MHz version
:r01=2366400068. -- serial number
:r02=66.
:r03=31994.
:r04=2411.
:r05=2525.
:r06=270.
:r07=2520.
:r08=2136.
:r09=0.
:r10=2970.
:r11=2328.
:r12=2502.
:r13=250.
:r14=2386.
:r15=2034.
:r16=0.
:r17=2982.
:r18=5050.
:r19=0.
:r20=1,1.       -- status of both outputs
:r21=3.         -- ch1 waveform
:r22=0.         -- ch2 waveform
:r23=1000000,0. -- ch1 frequency*100 and units
:r24=1000000,0. -- ch2 frequency*100 and units
:r25=5000.      -- (ch1 amplitude, V)*1000, V
:r26=5000.      -- (ch2 amplitude, V)*1000, V
:r27=1000.      -- (ch1 offset, V)*100 + 1000,  (1..1999)
:r28=1000.      -- (ch2 offset, V)*100 + 1000,  (1..1999)
:r29=500.       -- (ch1 duty cycle, %) * 10
:r30=500.       -- (ch2 duty cycle, %) * 10
:r31=0.         -- (ch2-ch1 phase, deg) * 10
:r32=0,0,0,0.   -- extension (pulse/burst, sweep1/sweep2, ?, ?)
:r33=0.         -- screen: 0-ch1, 1-ch2, 2,3-sys, 4-meas/meas,
                   5-meas/counter, 6-mod/sweep_ch1 ,7-mod/sweep_ch2,
                   8-mod/pulse_ch1, 9-mod-burst_ch1
:r34=0.         -- screen/field
:r35=0.
:r36=0.
:r37=100.      -- (meas gate, s)*100
:r38=0.        -- meas mode: 0-freq, 1-period
:r39=0.        -- ?
:r40=100000.   -- (start freq, Hz)*100
:r41=1000000.  -- (stop freq, Hz)*100
:r42=100.      -- (sweep time, s)*10
:r43=0.        -- sweep direction: 0-rise, 1-fall, 2-rise&fall
:r44=0.        -- sweep mode: 0-lin, 1-log
:r45=1000,0.   -- pulse W, ns
:r46=10000,0.  -- pulse T, ns
:r47=100.      -- pulse offset,% (0..120)
:r48=500.      -- (pulse amp, V)*100
:r49=5.        -- burst count + burst start?
:r50=0.        -- language?
:r51=0.
:w51/r52=0.    -- sound 0/1
:w52/r53=12.   -- brightness 0..12
:r54=0.
:r55=0,0,0,0,0. -- sync (freq, waveform, amp, offset, duty)
:r56=15.

*/

#include "drv_serial.h"

class Driver_serial_jds6600: public Driver_serial {
  Opt add_opts(const Opt & opts){
    opts.check_unknown({"dev", "timeout", "errpref", "idn"});
    Opt o(opts);
    o.put("speed", 115200); // baud rate
    o.put("parity", "8N1"); // character size, parity, stop bit
    o.put("cread",  1);     // always set cread=1
    o.put("clocal", 1);     // always set clocal=1
    o.put("sfc",    0);
    o.put("raw",    1);
    o.put("delay",0.1); // 100ms delay after write
    o.put("onlcr",  1); // convert \n -> \n\r on writing
    o.put("igncr",  1); // ignore \r on reading
    o.put("add_str",  "\n");
    o.put("trim_str", "\n");
    o.put("read_cond",  "always"); // always read answers

    // set defaults (only it no values are set by user)
    o.put_missing("timeout", 5.0);
    o.put_missing("errpref", "JDS6600: ");
    o.put_missing("idn", "JDS6600");

    return o;
  }
public:
  Driver_serial_jds6600(const Opt & opts): Driver_serial(add_opts(opts)){}
};

#endif
