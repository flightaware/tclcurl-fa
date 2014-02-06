package require TclCurl

curl::transfer -url "127.0.0.1" -httpversion none -verbose 1 -nobody 1

curl::transfer -url "127.0.0.1" -httpversion 1.0  -verbose 1 -nobody 1

curl::transfer -url "127.0.0.1" -httpversion 1.1  -verbose 1 -nobody 1






