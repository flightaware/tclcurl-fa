package require TclCurl


puts "Version [curl::versioninfo -version]"
puts "Version (num): [curl::versioninfo -versionnum]"
puts "Host: [curl::versioninfo -host]"
puts "Features: [curl::versioninfo -features]"
puts "SSL version: [curl::versioninfo -sslversion]"
puts "SSL version (num): [curl::versioninfo -sslversionnum]"
puts "libz version: [curl::versioninfo -libzversion]"
puts "Protocols [curl::versioninfo -protocols]"


