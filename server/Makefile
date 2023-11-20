PROGRAMS := device_d device_c

MOD_HEADERS := http_server.h dev_manager.h device.h tun.h\
               drv.h drv_spp.h drv_utils.h drv_test.h drv_usbtmc.h\
               drv_serial.h drv_net.h drv_gpib.h drv_vxi.h\
               drv_serial_tenma_ps.h drv_serial_asm340.h drv_serial_simple.h\
               drv_serial_vs_ld.h drv_net_gpib_prologix.h drv_serial_et.h

MOD_SOURCES := http_server.cpp dev_manager.cpp device.cpp tun.cpp\
               drv.cpp drv_utils.cpp drv_spp.cpp drv_usbtmc.cpp\
               drv_serial.cpp drv_net.cpp drv_gpib.cpp drv_vxi.cpp

SIMPLE_TESTS := dev_manager drv_spp drv_utils
OTHER_TESTS := device_d.test1\
               device_d.test2\
               device_d.test3\
               device_d.test4\
               device_d.test5

# use C++14 for shared locks
CXXFLAGS := -std=gnu++14
PKG_CONFIG := libmicrohttpd libcurl


# detect libgpib
# .SHELLSTATUS variable appears in make 4.2. We set it to 0 for earlier
# versions and switch off the detection
$(shell true)
ifneq (${.SHELLSTATUS},0)
  .SHELLSTATUS := 0
endif
$(shell pkg-config --exists libgpib)
ifeq (${.SHELLSTATUS},0)
PKG_CONFIG += libgpib
CXXFLAGS   += -DUSE_GPIB
endif

# VXI, remove if not needed.
# rpcgen program and libtirpc should be installed.
CXXFLAGS   += -DUSE_VXI

################

MODDIR := ../modules
include $(MODDIR)/Makefile.inc

## manpages
man: device_c.1 device_d.1
%.1: %
	./$* --pod | pod2man -n $* -c device2 -r device2 > $@

### vxi11 protocol
vxi11.h vxi11_clnt.c vxi11_svc.c vxi11_xdr.c: vxi11.x
	rpcgen $<
