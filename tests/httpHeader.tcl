package require TclCurl

curl::transfer -url "127.0.0.1" -verbose 1 -nobody 1 \
        -httpheader [list "hola: hello" "adios: goodbye"]








