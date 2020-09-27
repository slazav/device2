PROGRAMS := device_d device_c
MOD_HEADERS := http_server.h dev_manager.h drivers.h device.h
MOD_SOURCES := http_server.cpp dev_manager.cpp drivers.cpp device.cpp

SIMPLE_TESTS := dev_manager driver_spp
OTHER_TESTS := device_d.test1\
               device_d.test2\
               device_d.test3\
               device_d.test4

# use C++14 for shared locks
CXXFLAGS := -std=gnu++14
PKG_CONFIG := libmicrohttpd libcurl

MODDIR := ../modules
include $(MODDIR)/Makefile.inc




