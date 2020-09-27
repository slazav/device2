all:
	make -C server
	make -C client

prefix  ?= /usr
bindir  ?= $(prefix)/bin
sysconfdir ?= /etc
initdir    ?= $(sysconfdir)/init.d

install: all
	install -D -m755 server/device_d -t ${bindir}
	install -D -m755 client/device_c -t ${bindir}
	install -D devices.cfg -t ${sysconfdir}/device2/
	install -D device_d.init ${initdir}/device_d


