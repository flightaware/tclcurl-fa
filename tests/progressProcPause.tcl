package require TclCurl

# Another one of my contrived but working examples.

proc ProgressCallback {dltotal dlnow ultotal ulnow} {
    global i curlHandle
    
    set dltotal [expr int($dltotal)]
    set dlnow   [expr int($dlnow)]
    set ultotal [expr int($ultotal)]
    set ulnow   [expr int($ulnow)]

    puts "$i Progress callback: $dlnow of $dltotal downloaded"

    if {$i==10} {
        $curlHandle pause
    } elseif {$i==30} {
        $curlHandle resume
    }
    incr i

    return
}


set i 0

set curlHandle [curl::init]

$curlHandle configure -url "127.0.0.1/~andres/cosa&co.tar"  \
        -progressproc ProgressCallback -file cosa.tar -noprogress 0
$curlHandle perform

$curlHandle cleanup





