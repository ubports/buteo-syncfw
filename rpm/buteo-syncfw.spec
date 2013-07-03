Name: buteo-syncfw
Version: 0.6.1
Release: 1
Summary: Synchronization backend
Group: System/Libraries
URL: https://github.com/nemomobile/buteo-syncfw
License: LGPLv2.1
Source0: %{name}-%{version}.tar.gz
BuildRequires: doxygen, fdupes
BuildRequires: pkgconfig(QtCore)
BuildRequires: pkgconfig(dbus-1)
BuildRequires: pkgconfig(contextsubscriber-1.0)
BuildRequires: pkgconfig(accounts-qt)
BuildRequires: pkgconfig(libsignon-qt)
BuildRequires: pkgconfig(QtSystemInfo)
BuildRequires: libiphb-devel
Requires: %{name}-msyncd
# TODO: needs a proper fix
# Patch0: 0001-Synchronizer-removeProfile-remove-profiles-even-if-p.patch

%description
%{summary}.

%files
%defattr(-,root,root,-)
%config %{_sysconfdir}/buteo/*
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

# Should not be required anymore, sticks around for now
# in case we need to test something explicitely with qt4 stack
%package qt4-msyncd
Summary: Buteo sync daemon
Group: System/Libraries
Requires: %{name} = %{version}-%{release}
Requires: systemd
Requires: systemd-user-session-targets
Conflicts: buteo-syncfw-qt5-msyncd

%description qt4-msyncd
%{summary}.

%files qt4-msyncd
%defattr(-,root,root,-)
%config %{_libdir}/systemd/user/*.service
%{_libdir}/systemd/user/user-session.target.wants/*.service
%config %{_sysconfdir}/syncwidget/*
%{_bindir}/msyncd

%package doc
Summary: Documentation for %{name}
Group: Documentation

%description doc
%{summary}.

%files doc
%defattr(-,root,root,-)
%{_docdir}/sync-fw-doc/*

%package tests
Summary: Tests for %{name}
Group: Development/Libraries

%description tests
%{summary}.

%files tests
%defattr(-,root,root,-)
/opt/tests/buteo-syncfw/*.so
/opt/tests/buteo-syncfw/*.pl
/opt/tests/buteo-syncfw/*.sh
/opt/tests/buteo-syncfw/sync-fw-tests*
/opt/tests/buteo-syncfw/syncprofiletests
/opt/tests/buteo-syncfw/test-definition/*.xml
%{_datadir}/accounts/services/*.service


%prep
%setup -q


%build
qmake
make


%install
make INSTALL_ROOT=%{buildroot} install
chmod +x %{buildroot}/opt/tests/buteo-syncfw/*.pl %{buildroot}/opt/tests/buteo-syncfw/*.sh
%fdupes %{buildroot}/opt/tests/buteo-syncfw/
mkdir -p %{buildroot}%{_libdir}/systemd/user/user-session.target.wants
ln -s ../msyncd.service %{buildroot}%{_libdir}/systemd/user/user-session.target.wants/


%post
/sbin/ldconfig
if [ "$1" -ge 1 ]; then
    systemctl-user daemon-reload || true
    systemctl-user restart msyncd.service || true
fi

%postun
/sbin/ldconfig
if [ "$1" -eq 0 ]; then
    systemctl-user stop msyncd.service || true
    systemctl-user daemon-reload || true
fi
