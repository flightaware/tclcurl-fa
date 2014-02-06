package require TclCurl

if {$tcl_platform(platform)=="windows"} {
    set certFile [file join $env(windir) curl-ca-bundle.crt]
} else {
    set certFile /usr/local/share/curl/curl-ca-bundle.crt
}

::curl::transfer -url https://www.paypal.com/ -cainfo $certFile \
        -file paypal.html


puts "https://www.paypal.com/ saved in 'paypal.html'"
