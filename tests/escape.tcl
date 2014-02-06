package require TclCurl


set escaped [curl::escape {What about this?}]
puts "String to escape: What about this? - $escaped"
puts "And the reverse: [curl::unescape $escaped]"
