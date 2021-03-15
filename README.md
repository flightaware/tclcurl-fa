[![Linux CI](https://github.com/flightaware/tclcurl-fa/actions/workflows/linux-ci.yml/badge.svg)](https://github.com/flightaware/tclcurl-fa/actions/workflows/linux-ci.yml)
[![Mac CI](https://github.com/flightaware/tclcurl-fa/actions/workflows/mac-ci.yml/badge.svg)](https://github.com/flightaware/tclcurl-fa/actions/workflows/mac-ci.yml)
[![Windows CI](https://github.com/flightaware/tclcurl-fa/actions/workflows/windows-ci.yml/badge.svg)](https://github.com/flightaware/tclcurl-fa/actions/workflows/windows-ci.yml)

# TclCurl - get a URL with FTP, TELNET, LDAP, DICT, FILE, HTTP or HTTPS syntax.

## Introduction

TclCurl gives the Tcl programmer access to the facilities of libcurl. For more information
about what libcurl is capable of check http://curl.haxx.se/

To make type:

      ./configure ?--enable-threads?
      make
      make install

The configure script will deduce `$PREFIX` from the tcl installation.
The generated `Makefile` uses the file `$PREFIX/lib/tclConfig.sh` that was left by
the make of tcl for most of its configuration parameters.

There is another file, ReadMeW32.txt, to explain how to compile in Windows.

## Troubleshooting

If you are running Red Hat or Fedora and you are compiling from source,
make sure the directory `/usr/local/lib` is listed in `/etc/ld.so.conf`,
if it isn't add it before installing cURL.

If the configure script doesn't detect Tcl even though it is there, it
is probably because there is no `tclConfig.sh` file, maybe you need
to install a `tcl-devel` package or maybe it would be a good idea
to download the latest version, and install that.

## Usage

    package require TclCurl

    set curlHandle [curl::init]

    $curlHandle perform

    $curlHandle getinfo curlinfo_option

    $curlHandle duphandle

    $curlhandle cleanup

    curl::transfer

    curl::version

    curl::escape $url

    curl::unescape $url

    curl::curlConfig option

Check the man page for details.

## Authors

Originally written by:  Andres Garcia (fandom@telefonica.net)

Cloned from a repo by Steve Havelka once at:  https://bitbucket.org/smh377/tclcurl/

## License

Use at your own risk.  No support.
BSD like license, check 'license.terms' for details.
