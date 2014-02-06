package require TclCurl

set curlHandle [curl::init]

puts "First we save the headers in 'header.txt'"

$curlHandle configure -noprogress 1 -nobody 1 -url "127.0.0.1" \
        -writeheader header.txt
$curlHandle perform

puts "And now we dump them to the console"

$curlHandle configure -writeheader ""
$curlHandle perform

$curlHandle cleanup





