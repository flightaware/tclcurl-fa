package require TclCurl


proc Perform {multiHandle} {
    if {[catch {$multiHandle active} activeTransfers]} {
        puts "Error checking active transfers: $activeTransfers"
        return -1
    }
    if {[catch {$multiHandle perform} running]} {
        puts "Error: $running"
        return 1
    }
    return $running
}

proc StartTransfer {multiHandle} {
    while {1==1} {
        set runningTransfers [Perform $multiHandle]
        if {$runningTransfers>0} {
            after 500
        } else {
            break
        }
    }
}

puts "We create and configure the easy handles"

set curlEasyHandle1 [curl::init]
set curlEasyHandle2 [curl::init]

$curlEasyHandle1 configure -url http://127.0.0.1/~andres/ -file index.html
$curlEasyHandle2 configure -url http://127.0.0.1/ -file index2.html

puts "Creating the multi handle"

set curlMultiHandle [curl::multiinit]

puts "Adding easy handles to the multi one"

$curlMultiHandle addhandle $curlEasyHandle1
$curlMultiHandle addhandle $curlEasyHandle2

puts "We start the transfer"

StartTransfer $curlMultiHandle

puts "Transfer done, cleanning up"

$curlMultiHandle removehandle $curlEasyHandle1
$curlMultiHandle removehandle $curlEasyHandle2

$curlMultiHandle cleanup
$curlEasyHandle1 cleanup
$curlEasyHandle2 cleanup


