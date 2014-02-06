#!/usr/local/bin/wish8.4

package require TclCurl

proc ProgressCallback {dltotal dlnow ultotal ulnow} {

    set dltotal [expr round($dltotal)]
    set dlnow   [expr round($dlnow)]
    set ultotal [expr round($ultotal)]
    set ulnow   [expr round($ulnow)]

    puts "Progress callback: $dltotal - $dlnow - $ultotal - $ulnow"

    return
}

proc Perform {multiHandle} {
    if {[catch {$multiHandle active} activeTransfers]} {
        puts "Error checking active transfers: $activeTransfers"
        return -1
    }

    if {[catch {$multiHandle perform} running]} {
        puts "Error: $running"
        return -1
    }
    return $running
}

proc Transfer {multiHandle easyHandle} {
    global eventId
    set runningTransfers [Perform $multiHandle]
    if {$runningTransfers>0} {
        set eventId [after 200 "Transfer $multiHandle $easyHandle"]
    } else {
        puts "Were are done, cleaning up..."
        $multiHandle removehandle $easyHandle
        $easyHandle  cleanup
        $multiHandle cleanup
        puts "All done"
    }
    return
}

proc StartTransfer {} {

    set curlEasyHandle [curl::init]

    $curlEasyHandle configure -url "127.0.0.1/~andres/cosa&co.tar"          \
            -canceltransvarname cancel   -progressproc ProgressCallback\
            -file cosa.tar -noprogress 0

    set curlMultiHandle [curl::multiinit]
    $curlMultiHandle addhandle $curlEasyHandle

    after 100 "Transfer $curlMultiHandle $curlEasyHandle"

    return
}

proc StopTransfer {} {
    global cancel eventId

    puts "The download has been cancelled"

    set cancel 1

    return
}

set start [button .start -text Start -command StartTransfer]
set stop  [button .stop  -text Stop  -command StopTransfer]

pack $start $stop -side left -padx 10 -pady 10






