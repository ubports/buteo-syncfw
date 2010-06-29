Name: buteo-syncfw
Version: 0.4.9
Release: 1
Summary: Synchronization backend
Group: System/Libraries
License: Other
Source0: %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildRequires: doxygen, qt-devel
BuildRequires: dbus-devel
BuildRequires: contextkit-devel
BuildRequires: libaccounts-qt-devel
#BuildRequires: dsme-devel

%description
%{summary}.

%files
%defattr(-,root,root,-)
%config %{_sysconfdir}/sync/*
%{_bindir}/*
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


%prep
%setup -q

%build
qmake
make %{?_smp_mflags}


%install
rm -rf %{buildroot}
make INSTALL_ROOT=%{buildroot} install
chmod -x %{buildroot}/%{_includedir}/libsyncprofile/*.h
chmod +x %{buildroot}/%{_bindir}/*


%clean
rm -rf %{buildroot}


%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig
