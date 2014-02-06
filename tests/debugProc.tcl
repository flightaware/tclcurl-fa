package require TclCurl

proc DebugProc {infoType data} {

    switch $infoType {
        0 {
            set type "text"
        }
        1 {
            set type "incoming header"
        }
        2 {
            set type "outgoing header"
        }
        3 {
            set type "incoming data"
        }
        4 {
            set type "outgoing data"
        }
        5 {
            set type "incoming SSL data"
        }
        6 {
            set type "outgoing SSL data"
        }
    }

    puts "Type: $type - Data:"
    puts "$data"

    return 0
}

set curlHandle [curl::init]

$curlHandle configure -url 127.0.0.1 -verbose 1 \
        -debugproc DebugProc

$curlHandle perform
$curlHandle cleanup
