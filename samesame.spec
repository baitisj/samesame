Name: samesame
Version: 1.10
Release: 0
Group: Applications/File
Summary: Find identical files and optionally link them together
URL: http://samesame.kruijff.org/
Source: http://samesame.kruijff.org/%{name}-%{version}.tar.bz2
License: BSD (2 Clauses), Copyright (c) 2009 Alex de Kruijff
BuildRoot: %{_tmppath}/%{name}-%{version}-build
# BuildRequires: gcc-c++

%description
SameSame is a collection of tools that fall in to the category of file
management software. These tools will prevent that you need to delete
files or buy more disk space. Instead they solve low disk space problems
by linking identical files together and thus free up waisted disk space.

This collection was inspired by the application samefile written by Jens
Schweikhardt. The collection comes with its own version of samefile that
is noticeable faster and is able to process a much larger file list.

This port containt two set of application: the first are duplicate files
finder search for identical files and the second are duplicate file
removers perform some kind of action based on those results.

Typical usage would be: find / | samefile -i | samelink

This would search for identical files and clean up wasted disk space by
linking them together. If you prefer removing one of the identical file,
then you should replace samelink with samerm. You can add the option -vn
after both application for a verbose dry-run.

Please see the man page samesame for a introduction to all applications.

%prep
%setup

%build
%configure -disable-chtools
make
make check

%install
%makeinstall

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-, root, root, 0755)
%doc INSTALL README
%doc %{_mandir}/man1/samesame.1*  
%doc %{_mandir}/man1/samefile.1*  
%doc %{_mandir}/man1/samearchive-lite.1*  
%doc %{_mandir}/man1/samearchive.1*  
%doc %{_mandir}/man1/samechown.1*  
%doc %{_mandir}/man1/samecp.1*  
%doc %{_mandir}/man1/samedelay.1*  
%doc %{_mandir}/man1/sameln.1*  
%doc %{_mandir}/man1/samemv.1*  
%doc %{_mandir}/man1/samerm.1*  
  
%{_prefix}/bin/samefile  
%{_prefix}/bin/samearchive  
%{_prefix}/bin/samearchive-lite  
%{_prefix}/bin/samechown  
%{_prefix}/bin/samecp  
%{_prefix}/bin/samedelay  
%{_prefix}/bin/sameln  
%{_prefix}/bin/samemv  
%{_prefix}/bin/samerm  
