Name:         device2
Version:      0.1
Release:      alt1

Summary:      client-server system for accessing devices and programs in experimental setups
Group:        System
URL:          https://github.com/slazav/dev_server
License:      GPL

Packager:     Vladislav Zavjalov <slazav@altlinux.org>

Source:       %name-%version.tar
BuildRequires: gcc-c++
BuildRequires: libmicrohttpd-devel
BuildRequires: libjansson-devel
BuildRequires: libcurl-devel
BuildRequires: linux-gpib-devel

%description
Device2 is a client-server system for accessing devices and programs
in experimental setups.
device_d: server part, works with devices

%prep
%setup -q

%build
tar -xvf modules.tar
%make

%install
%makeinstall initdir=%buildroot%_initdir
mkdir -p %buildroot%_sharedstatedir/device_d

%post
%post_service device_d

%preun
%preun_service device_d

%files
%attr(2770,root,users) %dir %_sharedstatedir/device_d
%_bindir/device_d
%_bindir/device_c
%config %_initdir/device_d
%dir %_sysconfdir/device2
%config(noreplace) %_sysconfdir/device2/devices.cfg

%changelog
