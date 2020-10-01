PROGRAMS := device_d device_c

MOD_HEADERS := http_server.h dev_manager.h device.h tun.h\
               drv.h drv_spp.h drv_utils.h drv_test.h drv_usbtmc.h\
               drv_serial.h drv_net.h drv_gpib.h\
               drv_serial_tenma_ps.h drv_serial_asm340.h drv_serial_simple.h\
               drv_serial_vs_ld.h drv_net_gpib_prologix.h

MOD_SOURCES := http_server.cpp dev_manager.cpp device.cpp tun.cpp\
               drv.cpp drv_utils.cpp drv_spp.cpp drv_usbtmc.cpp\
               drv_serial.cpp drv_net.cpp drv_gpib.cpp

SIMPLE_TESTS := dev_manager drv_spp drv_utils
OTHER_TESTS := device_d.test1\
               device_d.test2\
               device_d.test3\
               device_d.test4\
               device_d.test5

# use C++14 for shared locks
CXXFLAGS := -std=gnu++14
PKG_CONFIG := libmicrohttpd libcurl libgpib

MODDIR := ../modules
include $(MODDIR)/Makefile.inc

## manpages
man: device_c.1 device_d.1
%.1: %
	./$* --pod | pod2man -n $* -c device2 -r device2 > $@



