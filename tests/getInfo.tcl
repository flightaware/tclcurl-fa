package require TclCurl

set curlHandle [curl::init]
$curlHandle configure -url "127.0.0.1" -filetime 1  -verbose 1             \
         -cookielist "127.0.0.1\tFALSE\t/\tFALSE\t1262307600\tad_browser_id\t  18864635"
$curlHandle perform

puts "Url: [$curlHandle getinfo effectiveurl]"
puts "Primary IP: [$curlHandle getinfo primaryip]"
puts "Primary port: [$curlHandle getinfo primaryport]"
puts "Local IP: [$curlHandle getinfo localip]"
puts "Local port: [$curlHandle getinfo localport]"
puts "Redirect url: [$curlHandle getinfo redirecturl]"
puts "Http-code: [$curlHandle getinfo responsecode]"
puts "Proxy response code: [$curlHandle getinfo httpconnectcode]"
set fileTime [$curlHandle getinfo filetime]
puts "Filetime: $fileTime - [clock format $fileTime]"
puts "Total time: [$curlHandle getinfo totaltime]"
puts "Name lookup time: [$curlHandle getinfo namelookuptime]"
puts "Name connect time: [$curlHandle getinfo connecttime]"
puts "Name pretransfer time: [$curlHandle getinfo pretransfertime]"
puts "Name start transfer time: [$curlHandle getinfo starttransfertime]"
puts "Name app connect time: [$curlHandle getinfo appconnecttime]"
puts "Name size upload: [$curlHandle getinfo sizeupload]"
puts "Name size download: [$curlHandle getinfo sizedownload]"
puts "Name speed download: [$curlHandle getinfo speeddownload]"
puts "Name speed upload: [$curlHandle getinfo speedupload]"
puts "Name header size: [$curlHandle getinfo headersize]"
puts "Name request size: [$curlHandle getinfo requestsize]"
puts "Name ssl verifyresult: [$curlHandle getinfo sslverifyresult]"
puts "SSL engines: [$curlHandle getinfo sslengines]"
puts "Name length download: [$curlHandle getinfo contentlengthdownload]"
puts "Name length upload: [$curlHandle getinfo contentlengthupload]"
puts "Content-Type: [$curlHandle getinfo contenttype]"
puts "Redirect time: [$curlHandle getinfo redirecttime]"
puts "Redirect count: [$curlHandle getinfo redirectcount]"
puts "Authentication methods available: [$curlHandle getinfo httpauthavail]"
puts "Authentication methods at the proxy: [$curlHandle getinfo proxyauthavail]"
puts "Operating System error number: [$curlHandle getinfo oserrno]"
puts "Number of successful connects: [$curlHandle getinfo numconnects]"
puts "Known cookies: [$curlHandle getinfo cookielist]"
set certList [$curlHandle getinfo certinfo]
set certNum  [lindex $certList 0]
puts "Nº de certificados: $certNum"
for {set i 1} {$i<=$certNum} {incr i} {
    puts [lindex $certList $i]
}

$curlHandle cleanup






