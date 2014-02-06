package require TclCurl

set curlHandle [curl::init]

puts "First it goes into error.txt"
$curlHandle configure -url  127.0.0.1/~andres/cosa&co.tar -stderr error.txt \
        -noprogress 0 -file cosa.tar

catch {$curlHandle perform}


puts "And then to stderr:"
$curlHandle configure -stderr ""

catch {$curlHandle perform}

$curlHandle cleanup





