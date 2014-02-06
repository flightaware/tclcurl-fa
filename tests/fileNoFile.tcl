package require TclCurl

set curlHandle [curl::init]

puts "First we save in 'index.html'"

$curlHandle configure -noprogress  1 -url "127.0.0.1" -file "index.html"
$curlHandle perform

puts "And now in stdout"

$curlHandle configure -file ""
$curlHandle perform

$curlHandle cleanup





