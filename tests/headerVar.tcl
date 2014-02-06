package require TclCurl

curl::transfer -url 127.0.0.1 -header 1 -nobody 1 -headervar headers

puts "The received headers"
foreach {key content} [array get headers] {
    puts "headers($key): $content"
}





