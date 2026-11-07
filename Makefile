
DESTDIR    ?=
prefix     ?= $(DESTDIR)/usr
bindir     ?= $(prefix)/bin
sysconfdir ?= $(DESTDIR)/etc
initdir    ?= $(sysconfdir)/init.d
tcldatadir ?= $(prefix)/share/tcl
man1dir    ?= $(prefix)/share/man/man1
unitdir    ?= $(prefix)/lib/systemd/system

myconfdir = ${sysconfdir}/device2/
mytcldir  = ${tcldatadir}/Device2/

srcdir = src

all:
	make -C ${srcdir}

man:
	make -C ${srcdir} man

install: all man
	install -D -m755 ${srcdir}/device_d -t ${bindir}
	install -D -m755 ${srcdir}/device_c -t ${bindir}
	install -D -m755 ${srcdir}/device_ping -t ${bindir}
	install -D -m644 *.cfg      -t ${myconfdir}
	install -D -m644 tcl/*.tcl  -t ${mytcldir}
	install -D -m644 ${srcdir}/*.1    -t ${man1dir}
	install -D -m755 device_d.init    -t ${initdir}
	install -D -m644 device_d.service -t ${unitdir}

clean:
	make -C server clean
	make -C modules clean
