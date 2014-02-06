# This script inst`pwd+alls TclCurl in a Windows system, you only
# have to double-click on it and, provided Tcl/Tk is already
# installed, TclCurl will be ready to use.
#
# Copyright (c) 2001-2011 Andres Garcia Garcia.
#
# See the file "license.terms" for information on usage and redistribution
# of this file, and for a DISCLAIMER OF ALL WARRANTIES.


wm withdraw .

set tclLibDir     [file dirname $tcl_library]
set tclBinDir     [file join [file dirname $tclLibDir] bin]
set windowsSystem [file join $env(windir) SYSTEM]

cd [file dirname [info script]]

foreach oldie [glob -nocomplain $tclLibDir/tclcurl*] {
    catch {file delete -force $oldie}
}

file copy -force tclcurl7.22.0 $tclLibDir

foreach sslFile [glob -nocomplain ssl/*] {
    catch {file copy -force $sslFile $windowsSystem}
}
foreach sslFile [glob -nocomplain certs/*] {
    file copy -force $sslFile [file join $env(windir) cacert.pem]
}

package forget TclCurl

if {[catch {package require TclCurl} version]} {
    tk_messageBox -icon error -type ok -title Error \
        -message "Install failed\n$version"
} else {
    tk_messageBox -icon info -type ok -title Installed \
        -message "Version $version installed"
}

exit
