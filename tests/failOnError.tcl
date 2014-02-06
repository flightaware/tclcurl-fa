package require TclCurl

set curlHandle [curl::init]
$curlHandle configure -url "127.0.0.1/cosa.html"

$curlHandle configure -failonerror 0 ; # This is the default
puts "With failonerror==0:"
catch {$curlHandle perform}
puts "\n\n\n\n\n"
puts "With failonerror==1:"
$curlHandle configure -failonerror 1
catch {$curlHandle perform}

$curlHandle cleanup



