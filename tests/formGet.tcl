package require TclCurl

curl::transfer -url "http://www.google.com/search?q=TclCurl&hl=en&btnG=Google+Search+&lr=" \
        -file tclcurl.html -cookiejar [file join [file dirname [info script]] cookieJar.txt]
puts "Transfer saved in 'tclcurl.html'"
puts "Cookies en el fichero: [file join [file dirname [info script]] cookieJar.txt]"







