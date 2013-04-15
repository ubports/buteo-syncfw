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
BuildRequires: pkgconfig(qttracker)
BuildRequires: pkgconfig(libsignon-qt)
BuildRequires: pkgconfig(QtSystemInfo)
BuildRequires: libiphb-devel
# TODO: needs a proper fix
# Patch0: 0001-Synchronizer-removeProfile-remove-profiles-even-if-p.patch

%description
%{summary}.

%files
%defattr(-,root,root,-)
%config %{_sysconfdir}/buteo/*
%config %{_libdir}/systemd/user/*.service
%config %{_sysconfdir}/syncwidget/*
%{_bindir}/msyncd
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


%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig
