package require TclCurl

set curlHandle [curl::init]
$curlHandle configure -url "http://127.0.0.1" -filetime 1 -ipresolve v4 \
        -verbose 1 -nobody 1
$curlHandle perform

$curlHandle cleanup






