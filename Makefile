all:
	make -C server

prefix     ?= /usr
bindir     ?= $(prefix)/bin
sysconfdir ?= /etc
initdir    ?= $(sysconfdir)/init.d
tcldatadir ?= $(prefix)/share/tcl
man1dir    ?= $(prefix)/share/man/man1

myconfdir = ${sysconfdir}/device2/
mytcldir  = ${tcldatadir}/Device2/

srcdir = server

man:
	make -C ${srcdir} man

install: all man
	install -D -m755 ${srcdir}/device_d -t ${bindir}
	install -D -m755 ${srcdir}/device_c -t ${bindir}
	install -D -m755 device_d.init ${initdir}/device_d
	install -D -m644 *.cfg      -t ${myconfdir}
	install -D -m644 tcl/*.tcl  -t ${mytcldir}
	install -D -m644 ${srcdir}/*.1 -t ${man1dir}

clean:
	make -C server clean
	make -C modules clean
