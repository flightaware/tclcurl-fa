package require TclCurl

curl::transfer -url "http://127.0.0.1" -encoding deflated -verbose 1

curl::transfer -url "http://127.0.0.1" -encoding all      -verbose 1




