package require TclCurl

if {![file exists /home/andres/.getleft/cookies]} {
    puts "The given cookie file doesn't exist"
}

curl::transfer -url 127.0.0.1 \
        -cookiefile "/home/andres/.getleft/cookies" -verbose 1





