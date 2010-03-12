%define reldate 20100312
%define reltype C
# may be one of: C (current), R (release), S (stable)

Name: smapi
Version: 2.5.%{reldate}%{reltype}
Release: 1
Group: Libraries/FTN
Summary: Squish Messagebase API
URL: http://husky.sf.net
License: GPL
Requires: huskylib >= 1.9
Source: %{name}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-root

%description
Squish Messagebase API library for the Husky Project software.

%prep
%setup -q -n %{name}

%build
make

%install
rm -rf %{buildroot}
make DESTDIR=%{buildroot} install
cp cvsdate.h %{buildroot}%{_includedir}/smapi

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root)
%{_prefix}/*

