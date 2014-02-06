package require TclCurl

set curlHandle [curl::init]

$curlHandle configure -url 127.0.0.1 -verbose 1 -nobody 1


puts "First one is verbose"
$curlHandle perform

puts "The second isn't"
$curlHandle configure -verbose 0 
$curlHandle perform


$curlHandle cleanup







