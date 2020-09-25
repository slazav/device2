all:
	make -C server
	make -C client

prefix  ?= /usr
bindir  ?= $(prefix)/bin
sysconfdir ?= /etc
initdir    ?= $(sysconfdir)/init.d

install: all
	mkdir -p ${bindir} ${initdir}
	install server/dev_server ${bindir}
	install client/device_c ${bindir}
	install dev_server.cfg ${sysconfdir}
	install dev_server.init ${initdir}/dev_server


