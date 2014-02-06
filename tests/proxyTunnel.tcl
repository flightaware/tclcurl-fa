package require TclCurl

curl::transfer -url http://curl.haxx.se -verbose 1 \
        -proxy "192.168.0.0:8080" -httpproxytunnel 1







