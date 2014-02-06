package require TclCurl

proc ProgressCallback {dltotal dlnow ultotal ulnow} {

    set dltotal [expr int($dltotal)]
    set dlnow   [expr int($dlnow)]
    set ultotal [expr int($ultotal)]
    set ulnow   [expr int($ulnow)]

    puts "Progress callback: $dltotal - $dlnow - $ultotal - $ulnow"

    return
}


set curlHandle [curl::init]

$curlHandle configure -url "127.0.0.1/~andres/cosa&co.tar"  \
        -progressproc ProgressCallback -file cosa.tar -noprogress 0
$curlHandle perform

$curlHandle cleanup





