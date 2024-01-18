Name:         device2
Version:      1.8
Release:      alt1

Summary:      client-server system for accessing devices and programs in experimental setups
Group:        System
URL:          https://github.com/slazav/device2
License:      GPL

Packager:     Vladislav Zavjalov <slazav@altlinux.org>

Source:       %name-%version.tar
BuildRequires: gcc-c++
BuildRequires: libmicrohttpd-devel
BuildRequires: libjansson-devel
BuildRequires: libcurl-devel
BuildRequires: linux-gpib-devel

%description
Device2 is a client-server system for accessing devices and programs
in experimental setups.

%prep
%setup -q

%build
tar -xvf modules.tar
%make

%install
%makeinstall initdir=%buildroot%_initdir
mkdir -p %buildroot%_sharedstatedir/device_d


%post
%post_service device_d

%preun
%preun_service device_d

%files
%attr(2770,root,users) %dir %_sharedstatedir/device_d
%_bindir/device_d
%_bindir/device_c
%config %_initdir/device_d
%dir %_sysconfdir/device2
%config(noreplace) %_sysconfdir/device2/devices.cfg
%config(noreplace) %_sysconfdir/device2/device_c.cfg
%config(noreplace) %_sysconfdir/device2/device_d.cfg
%_tcldatadir/Device2
%_man1dir/device*

%changelog
* Thu Jan 18 2024 Vladislav Zavjalov <slazav@altlinux.org> 1.8-alt1
v1.8
- Systemd service: add WantedBy=multi-user.target
  (this is needed to start the server on boot)
- vxi: important bugfixes: fix for multi-thread usage,
  fix memory leak and uninitialised values in read(), thanks to valgrind

* Fri Nov 24 2023 Vladislav Zavjalov <slazav@altlinux.org> 1.7-alt1
v1.7
- Add vxi driver
- deb: simplify deb/rules, fix location of tcl library, add conffiles
- update Readme.md, add SPP protocol description
- update modules

* Tue Nov 14 2023 Vladislav Zavjalov <slazav@altlinux.org> 1.6-alt1
v1.6 a few fixes for Debian/Ubuntu build

* Wed Mar 22 2023 Vladislav Zavjalov <slazav@altlinux.org> 1.5-alt1
v1.5
- device_d server start: do not fail if pid-file exists but the process does not
- Revert "unquote messages for all drivers except spp" (introduced in v1.4)
- new action: close -- close device (it will be reopened if needed)
- drv_serial: add flush_on_err parameter, update comments and Readme.md
- drv_serial_tenma_ps: turn off SFC. This is important if status byte is zero
- drv_net: -open_delay option, wait before opening connection. May be needed for Siglent devices
- drv_net: add -delay option -- waiting after write command (default 0)
- update mapsoft2-lib modules

* Sun Jun 19 2022 Vladislav Zavjalov <slazav@altlinux.org> 1.4-alt1
v1.4
- Unquote messages for all drivers except spp
- new driver drv_serial_l300 -- driver for Phoenix/Leybold L300i leak detector
- fix driver drv_serial_asm340 -- driver for Agilent VS leak detector 
- driver drv_usbtmc: add -delay option -- waiting after write command (default 0.01)
- drv_spp: modify error message for unexpected EOF in the stream

* Sun Apr 18 2021 Vladislav Zavjalov <slazav@altlinux.org> 1.3-alt1
v1.3
- device_c:
  - use read_words library for `use_dev` command (allow multi-line and quoted strings)
  - allow multiple argments in `ask` command
  - options can appear only before a non-option argument
- drivers:
  - add serial_et driver for EastTester devices
  - drv_serial: fix reading in non-blocking mode
  - drv_gpib: -delay parameter
  - drv_gpib: argument of -eos_mode option is any combination of X R B flags
- unlock action: do not return error if device is not locked
- update Readme.md

* Wed Mar 03 2021 Vladislav Zavjalov <slazav@altlinux.org> 1.2-alt1
v1.2 (a few important bugfixes)
- server:
  - Fix non-working --addr option. Server was always listening all addresses.
  - Print client address when opening a connection (with log level >= 2).
  - When stopping a server (--stop option) wait until the process exits.
  - Set umask 022 in daemon mode.
  - Fix locking in read-write operations to avoid mixing answers for different clients.
  - Fix gcc10 builds with old and new libmicrohttpd (MHD_Result enum appeares in 0x00097002).
  - Add switching to a different user (--user option).
  - Simplify initscript: use device_d options instead of start_daemon.
- drivers:
  - drv_usbtmc: fix error in -idn parameter handling
  - drv_gpib, drv_net, drv_usbtmc: add -read_cond parameter with default: qmark1w
- tcl:
  - In the legacy Device interface return list of lines instead of text data.
  - Fix error in Device::lock command.
- add UDEV.md file with some examples.
- update Readme.md.

* Sun Nov 15 2020 Vladislav Zavjalov <slazav@altlinux.org> 1.1-alt1
v1.1:
- drv_gpib: clear device buffers after opening
- drv_usbtmc: fix reading output from some slow operations
- device_c: release device after a single ask command

* Fri Oct 02 2020 Vladislav Zavjalov <slazav@altlinux.org> 1.0-alt1
- v1.0 (device_d, device_c, tcl library).
- Tested drivers: net (with LXI devices), gpib, usbtmc,
                  serial_asm340, serial_tenma_ps.
- Not tested: serial_vs_ld, serial_simple, net_gpib_prologix.
