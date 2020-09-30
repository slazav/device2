all:
	make -C server

prefix     ?= /usr
bindir     ?= $(prefix)/bin
sysconfdir ?= /etc
initdir    ?= $(sysconfdir)/init.d

mycofdir = ${sysconfdir}/device2/

install: all
	install -D -m755 server/device_d -t ${bindir}
	install -D -m755 server/device_c -t ${bindir}
	install -D -m644 devices.cfg  -t ${mycofdir}
	install -D -m644 device_c.cfg -t ${mycofdir}
	install -D -m644 device_d.cfg -t ${mycofdir}
	install -D -m755 device_d.init ${initdir}/device_d
