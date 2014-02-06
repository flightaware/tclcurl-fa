package require TclCurl

puts "The error string for code 6 is '[curl::easystrerror 6]'"

puts "If the number is too big: '[curl::easystrerror 2000]'"

puts "It works the same way for the multi interface: '[curl::multistrerror 1]'"
puts "And the share interface: '[curl::sharestrerror 1]'"

catch {curl::easystrerror frelled} errorMsg
puts "And if we use a nonsensical code: '$errorMsg'"

catch {curl::sharestrerror} errorMsg
puts "And if we forget the error code:\n    '$errorMsg'"
