package require TclCurl

curl::transfer -url "127.0.0.1" -filetime 1                                  \
        -infoeffectiveurl    effectiveUrl  -inforesponsecode    httpCode     \
        -infofiletime        fileTime      -infototaltime       totalTime    \
        -infonamelookuptime  nameLookUpTime                                  \
        -infoconnecttime     connectTime   -infopretransfertime preTime      \
        -infostarttransfertime startTransferTime                             \
        -infosizeupload      sizeUpload    -infosizedownload    sizeDownload \
        -infospeeddownload   speedDownload -infospeedupload     speedUpload  \
        -infoheadersize      headerSize    -inforequestsize     requestSize  \
        -infosslverifyresult sslVerifyResult                                 \
        -infocontentlengthupload  contentLengthUpload                        \
        -infocontentlengthdownload contentLengthDownload                     \
        -infocontenttype     contentType                                     \
        -inforedirecttime    redirectTime                                    \
        -inforedirectcount   redirectCount


puts "Url: $effectiveUrl"
puts "Response code: $httpCode"
puts "Filetime: $fileTime - [clock format $fileTime]"
puts "Total time: $totalTime"
puts "Name lookup time: $nameLookUpTime"
puts "Name connect time: $connectTime"
puts "Name pretransfer time: $preTime"
puts "Name start transfer time: $startTransferTime"
puts "Name size upload: $sizeUpload"
puts "Name size download: $sizeDownload"
puts "Name speed download: $speedDownload"
puts "Name speed upload: $speedUpload"
puts "Name header size: $headerSize"
puts "Name request size: $requestSize"
puts "Name ssl verifyresult: $sslVerifyResult"
puts "Name length download: $contentLengthDownload"
puts "Name length upload: $contentLengthUpload"
puts "Content-Type: $contentType"
puts "Redirect time: $redirectTime"
puts "Redirect count: $redirectCount"








