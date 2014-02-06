package require TclCurl

set curlHandle [curl::init]

# This just checks the server for messages, if you want to download them
# you need: -url "pop3://pop3.telefonica.net:110/1",  -url "pop3://pop3.telefonica.net:110/2", etc

$curlHandle configure -url "pop3://pop3.telefonica.net:110"

$curlHandle configure -username "fandom\$telefonica.net"
$curlHandle configure -password "XXXXXXXX"

$curlHandle configure -bodyvar recieved

$curlHandle configure -verbose 1

$curlHandle perform

$curlHandle cleanup

puts "Recieved:"
puts \n$recieved\n\n




