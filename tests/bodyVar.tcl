package require TclCurl

# These tests has some urls that don't exists outside my system,
# so IT WON'T WORK FOR YOU unless you change them.

set curlHandle [curl::init]
$curlHandle configure -url "127.0.0.1" -bodyvar body -noprogress 1 
$curlHandle perform

$curlHandle configure -url "127.0.0.1/~andres/" -bodyvar newBody
$curlHandle perform

$curlHandle cleanup

puts "First page:"
puts $body

puts "Second page:"
puts $newBody

# You can also use it for binary transfers

curl::transfer \
        -url {127.0.0.1/~andres/HomePage/getleft/images/getleft.png} \
        -bodyvar image -noprogress 1 -verbose 1

if [catch {open "getleft.png" w} out] {
    puts "Could not open $out."
    exit
}

fconfigure $out -translation binary
puts  $out $image
close $out


