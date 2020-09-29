PROGRAMS := device_d device_c

MOD_HEADERS := http_server.h dev_manager.h device.h\
               drv.h drv_spp.h drv_test.h drv_usbtmc.h\
               drv_serial.h drv_net.h\
               drv_serial_tenma_ps.h drv_serial_asm340.h drv_serial_simple.h

MOD_SOURCES := http_server.cpp dev_manager.cpp device.cpp\
               drv.cpp drv_spp.cpp drv_usbtmc.cpp\
               drv_serial.cpp drv_net.cpp

SIMPLE_TESTS := dev_manager drv_spp
OTHER_TESTS := device_d.test1\
               device_d.test2\
               device_d.test3\
               device_d.test4\
               device_d.test5

# use C++14 for shared locks
CXXFLAGS := -std=gnu++14
PKG_CONFIG := libmicrohttpd libcurl

MODDIR := ../modules
include $(MODDIR)/Makefile.inc




