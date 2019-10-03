Name:    buteo-syncfw-qt5
Version: 0.8.16
Release: 1
Summary: Synchronization backend
Group:   System/Libraries
URL:     https://git.sailfishos.org/mer-core/buteo-syncfw/
License: LGPLv2
Source0: %{name}-%{version}.tar.gz
Source1: %{name}.privileges
BuildRequires: doxygen, fdupes
BuildRequires: pkgconfig(Qt5Core)
BuildRequires: pkgconfig(Qt5Network)
BuildRequires: pkgconfig(Qt5DBus)
BuildRequires: pkgconfig(Qt5Sql)
BuildRequires: pkgconfig(Qt5Test)
BuildRequires: pkgconfig(dbus-1)
BuildRequires: pkgconfig(accounts-qt5) >= 1.13
BuildRequires: pkgconfig(libsignon-qt5)
BuildRequires: pkgconfig(libiphb)
BuildRequires: pkgconfig(qt5-boostable)
BuildRequires: pkgconfig(keepalive)
BuildRequires: pkgconfig(gio-2.0)
BuildRequires: pkgconfig(mce-qt5) >= 1.1.0
BuildRequires: doxygen
Requires: mapplauncherd-qt5
Requires: glib2
Requires: libmce-qt5 >= 1.1.0

%description
%{summary}.

%files
%defattr(-,root,root,-)
%{_libdir}/*.so.*

%package devel
Summary: Development files for %{name}
Group: Development/Libraries
Requires: %{name} = %{version}-%{release}

%description devel
%{summary}.

%files devel
%defattr(-,root,root,-)
%{_includedir}/*
%{_libdir}/*.so
%{_libdir}/*.prl
%{_libdir}/pkgconfig/*.pc

%package msyncd
Summary: Buteo sync daemon
Group: System/Libraries
Requires: %{name} = %{version}-%{release}
Requires: systemd
Requires: systemd-user-session-targets
Provides: buteo-syncfw-msyncd = %{version}
Obsoletes: buteo-syncfw-msyncd < %{version}

%description msyncd
%{summary}.

%files msyncd
%defattr(-,root,root,-)
%{_libdir}/systemd/user/*.service
%{_libdir}/systemd/user/user-session.target.wants/*.service
%{_sysconfdir}/syncwidget
%{_bindir}/msyncd
%{_datadir}/mapplauncherd/privileges.d/*
%{_datadir}/glib-2.0/schemas/*
%dir %{_libdir}/buteo-plugins-qt5
%dir %{_libdir}/buteo-plugins-qt5/oopp

%package doc
Summary: Documentation for %{name}
Group: Documentation

%description doc
%{summary}.

%files doc
%defattr(-,root,root,-)
%{_docdir}/buteo-syncfw-doc

%package tests
Summary: Tests for %{name}
Group: Development/Libraries

%description tests
%{summary}.

%files tests
%defattr(-,root,root,-)
/opt/tests/buteo-syncfw
%{_datadir}/accounts/services/*.service


%prep
%setup -q


%build
%qmake5 -recursive "VERSION=%{version}" CONFIG+=usb-moded DEFINES+=USE_KEEPALIVE
make %{_smp_mflags}
make doc %{_smp_mflags}


%install
make INSTALL_ROOT=%{buildroot} install
chmod +x %{buildroot}/opt/tests/buteo-syncfw/*.pl %{buildroot}/opt/tests/buteo-syncfw/*.sh
%fdupes %{buildroot}/opt/tests/buteo-syncfw/
mkdir -p %{buildroot}%{_libdir}/systemd/user/user-session.target.wants
ln -s ../msyncd.service %{buildroot}%{_libdir}/systemd/user/user-session.target.wants/

mkdir -p %{buildroot}%{_datadir}/mapplauncherd/privileges.d
install -m 644 -p %{SOURCE1} %{buildroot}%{_datadir}/mapplauncherd/privileges.d/
mkdir -p %{buildroot}%{_libdir}/buteo-plugins-qt5/oopp

%post
/sbin/ldconfig
if [ "$1" -ge 1 ]; then
    systemctl-user daemon-reload || true
    systemctl-user try-restart msyncd.service || true
fi

%post msyncd
glib-compile-schemas %{_datadir}/glib-2.0/schemas

%postun
/sbin/ldconfig
if [ "$1" -eq 0 ]; then
    systemctl-user stop msyncd.service || true
    systemctl-user daemon-reload || true
fi
