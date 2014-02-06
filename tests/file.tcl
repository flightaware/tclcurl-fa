package require TclCurl

set curlHandle [curl::init]
$curlHandle configure -url "127.0.0.1" -file "index.html"
$curlHandle perform

puts "First transfer finished\n"

$curlHandle configure -header  1 -file cosa.html
$curlHandle perform

$curlHandle cleanup





