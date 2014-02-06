package require TclCurl

puts "cURL is installed in: [curl::curlConfig -prefix]"

set compiledOptions [curl::curlConfig -feature]
regsub -all {\n} $compiledOptions { - } compiledOptions
puts "The compiled options: $compiledOptions"

puts "The version in hex: [curl::curlConfig -vernum]"

puts "The built-in path to the CA cert bundle:\n\t[curl::curlConfig -ca]"

