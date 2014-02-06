package require TclCurl

set curlHandle1 [curl::init]

$curlHandle1 configure -url 127.0.0.1

set curlHandle2 [$curlHandle1 duphandle]

$curlHandle1 configure -url 127.0.0.1/~andres/

$curlHandle2 perform


$curlHandle1 cleanup
$curlHandle2 cleanup

