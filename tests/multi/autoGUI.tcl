#!/usr/local/bin/wish8.4

package require TclCurl

proc CleanUp {multiHandle easyHandle} {
    puts "\n\nCleaning up $multiHandle - $easyHandle\n\n"

    $multiHandle removehandle $easyHandle
    $multiHandle cleanup
    $easyHandle  cleanup

    puts "\n\nAll done\n\n"

    exit
}

proc StartTransfer {} {
    set multiHandle [curl::multiinit]
    set easyHandle  [curl::init]

    $easyHandle  configure -url http://127.0.0.1/~andres/HomePage.tar.gz -file home.tar.gz

    $multiHandle addhandle $easyHandle

    $multiHandle auto -command "CleanUp $multiHandle $easyHandle"
}

set start [button .start -text Start -command StartTransfer]
set stop  [button .stop  -text Stop  -command StopTransfer]

pack $start $stop -side left -padx 10 -pady 10
