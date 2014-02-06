package require TclCurl

puts "nobody==1 --- header==1"
curl::transfer -url "ftp://127.0.0.1/pub/indust2.gif" -nobody 1 -header 1

puts "nobody==0 --- header ignored"
curl::transfer -url "ftp://127.0.0.1/pub/indust2.gif" -nobody 0 \
        -file cosa.gif

puts "nobody==1 --- HEADERS==0"
curl::transfer -url "ftp://127.0.0.1/pub/indust2.gif" -nobody 1 \
        -header 0 -postquote [list "mkdir nada"]










