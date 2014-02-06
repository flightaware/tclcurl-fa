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

set curlEasyHandle [curl::init]

$curlEasyHandle configure -url http://127.0.0.1/ -file index.html

if {[catch {curl::multiinit} curlMultiHandle]} {
    puts "Error with multi handle init"
}

puts "The multi handle: $curlMultiHandle"

puts -nonewline "We add the easy handle: "
puts [$curlMultiHandle addhandle $curlEasyHandle]

StartTransfer $curlMultiHandle

puts "Calling getinfo [$curlMultiHandle getinfo]"

puts -nonewline "Removing the easy handle: "
puts [$curlMultiHandle removehandle $curlEasyHandle]

puts -nonewline "Cleanup the multi handle handle: "
puts [$curlMultiHandle cleanup]

$curlEasyHandle cleanup


