all:
	make -C server

prefix     ?= /usr
bindir     ?= $(prefix)/bin
sysconfdir ?= /etc
initdir    ?= $(sysconfdir)/init.d
tcldatadir ?= $(prefix)/share/tcl

myconfdir = ${sysconfdir}/device2/
mytcldir  = ${tcldatadir}/Device2/

install: all
	install -D -m755 server/device_d -t ${bindir}
	install -D -m755 server/device_c -t ${bindir}
	install -D -m755 device_d.init ${initdir}/device_d
	install -D -m644 *.cfg      -t ${myconfdir}
	install -D -m644 tcl/*.tcl  -t ${mytcldir}
