Name:         dev_server
Version:      0.1
Release:      alt1

Summary:      HTTP server for accessing devices
Group:        System
URL:          https://github.com/slazav/dev_server
License:      GPL

Packager:     Vladislav Zavjalov <slazav@altlinux.org>

Source:       %name-%version.tar
BuildRequires: libmicrohttpd-devel
Requires:      libmicrohttpd

%description
dev_server -- HTTP server for accessing devices

%prep
%setup -q

%build
tar -xvf modules.tar
%make

%install
%makeinstall initdir=%buildroot%_initdir
mkdir -p %buildroot%_sharedstatedir/dev_server

%post
%post_service dev_server

%preun
%preun_service dev_server

%files
%attr(2770,root,users) %dir %_sharedstatedir/dev_server
%_bindir/dev_server
%_bindir/device_c
%config %_initdir/dev_server
%config(noreplace) %_sysconfdir/dev_server.cfg

%changelog
