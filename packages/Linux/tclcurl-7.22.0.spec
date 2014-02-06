Name: tclcurl
Version: 7.22.0
Release: 1
License: BSD revised
Group:  Development/Libraries
Packager: Andres Garcia <fandom@telefonica.net>
Vendor: Fandom Enterprise
Source: http://personal1.iddeo.es/andresgarci/tclcurl/download/tarball/tclcurl-7.22.0.tar.gz
Summary: A Tcl interface for libcurl.

%description

TclCurl provides a binding to libcurl, with it you can upload
and download files using FTP, FTPS, HTTP, HTTPS, SCP, SFTP, TFTP, TELNET, DICT, FILE, LDAP,
LDAPS, IMAP, IMAPS, POP, POP3, SMTP, SMTPS and gopher syntax.

%prep
%setup

%build
./configure --mandir=/usr/share/man --prefix=/usr
make

%install
make install
mkdir -p /usr/share/doc/tclcurl-7.22.0/tests
cp -f ReadMe.txt               /usr/share/doc/tclcurl-7.22.0/
cp -f license.terms            /usr/share/doc/tclcurl-7.22.0/
cp -f doc/tclcurl.html         /usr/share/doc/tclcurl-7.22.0/
cp -f doc/tclcurl_multi.html   /usr/share/doc/tclcurl-7.22.0/
cp -f doc/tclcurl_share.html   /usr/share/doc/tclcurl-7.22.0/
cp -f doc/aolserver.txt        /usr/share/doc/tclcurl-7.22.0/
cp -f tests/*.tcl              /usr/share/doc/tclcurl-7.22.0/tests

%files
/usr/lib/TclCurl7.22.0/
/usr/share/doc/tclcurl-7.22.0/
/usr/share/man/mann/tclcurl.n.gz
/usr/share/man/mann/tclcurl_multi.n.gz
/usr/share/man/mann/tclcurl_share.n.gz


