all:
	make -C server

install: all
	mkdir -p ${bindir} ${initdir}
	install server/dev_server ${bindir}
	install dev_server.cfg ${sysconfdir}
	install dev_server.init ${initdir}/dev_server


