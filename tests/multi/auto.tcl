#!/usr/local/bin/wish8.4

package require TclCurl

#wm withdraw .

proc CleanUp {multiHandle easyHandle} {
    puts "\n\nCleaning up\n\n"

    $::multiHandle removehandle $::easyHandle
    $::multiHandle cleanup
    $::easyHandle  cleanup

    puts "\n\nAll done\n\n"

    exit
}

set multiHandle [curl::multiinit]
set easyHandle  [curl::init]

$easyHandle  configure -url http://127.0.0.1/~andres/HomePage.tar.gz -file home.tar.gz

$multiHandle addhandle $easyHandle

puts "Starting transfer..."

$multiHandle auto -command "CleanUp $multiHandle $easyHandle"



