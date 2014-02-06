# The share interface support is not yet completely done, since
# you can't use it with the multi interface.

package require TclCurl

set sHandle [curl::shareinit]
$sHandle share dns

set easyHandle1 [curl::init]
set easyHandle2 [curl::init]

$easyHandle1 configure -url http://127.0.0.1/         -share $sHandle
$easyHandle2 configure -url http://127.0.0.1/~andres/ -share $sHandle

$easyHandle1 perform
$easyHandle2 perform

$easyHandle1 cleanup
$easyHandle2 cleanup

$sHandle unshare dns
$sHandle cleanup

