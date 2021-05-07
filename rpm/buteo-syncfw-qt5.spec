Name:    buteo-syncfw-qt5
Version: 0.10.0
Release: 1
Summary: Synchronization backend
URL:     https://git.sailfishos.org/mer-core/buteo-syncfw/
License: LGPLv2
Source0: %{name}-%{version}.tar.gz
Source1: %{name}.privileges
BuildRequires: doxygen
BuildRequires: fdupes
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
BuildRequires: systemd
Requires: mapplauncherd-qt5
Requires: glib2
Requires: libmce-qt5 >= 1.1.0

%description
%{summary}.

%files
%defattr(-,root,root,-)
%{_libdir}/libbuteosyncfw5.so.*
%{_libexecdir}/buteo-oopp-runner

%package devel
Summary: Development files for %{name}
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
Requires: %{name} = %{version}-%{release}
Requires: systemd
Requires: systemd-user-session-targets
Provides: buteo-syncfw-msyncd = %{version}
Obsoletes: buteo-syncfw-msyncd < %{version}

%description msyncd
%{summary}.

%files msyncd
%defattr(-,root,root,-)
%{_userunitdir}/*.service
%{_userunitdir}/user-session.target.wants/*.service
%{_sysconfdir}/syncwidget
%{_bindir}/msyncd
%{_datadir}/mapplauncherd/privileges.d/*
%{_datadir}/glib-2.0/schemas/*
%dir %{_libdir}/buteo-plugins-qt5
%dir %{_libdir}/buteo-plugins-qt5/oopp

%package doc
Summary: Documentation for %{name}

%description doc
%{summary}.

%files doc
%defattr(-,root,root,-)
%{_docdir}/buteo-syncfw-doc

%package tests
Summary: Tests for %{name}

%description tests
%{summary}.

%files tests
%defattr(-,root,root,-)
/opt/tests/buteo-syncfw
%{_datadir}/accounts/services/*.service


%prep
%setup -q


%build
%qmake5 -recursive "VERSION=%{version}" CONFIG+=usb-moded DEFINES+=USE_KEEPALIVE DEFINES+=HAVE_BLUEZ_5
make %{_smp_mflags}
make doc %{_smp_mflags}


%install
make INSTALL_ROOT=%{buildroot} install
%fdupes %{buildroot}/opt/tests/buteo-syncfw/
mkdir -p %{buildroot}%{_userunitdir}/user-session.target.wants
ln -s ../msyncd.service %{buildroot}%{_userunitdir}/user-session.target.wants/

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
