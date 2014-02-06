package require TclCurl

# As an example this is contrived, but it works.

set alreadySent 0
set mailToSend \
"Date: Mon, 12 Sep 2011 20:34:29 +0200
To: fandom@telefonica.net
From: andres@verot.com
Subject: SMTP example

The body of the message starts here.

It could be a lot of lines, could be MIME encoded, whatever.
Check RFC5322.
"

proc sendString {size} {
    global alreadySent mailToSend
    
    set toSend       [string range $mailToSend $alreadySent [incr $alreadySent $size]]
    
    incr alreadySent [string length $toSend]

    return $toSend
}

set curlHandle [curl::init]

$curlHandle configure -url "smtp://smtp.telefonica.net:25"

$curlHandle configure -username "fandom\$telefonica.net"
$curlHandle configure -password "XXXXXXXX"

$curlHandle configure -mailfrom "fandom@telefonica.net"
$curlHandle configure -mailrcpt [list "fandom@telefonica.net" "andresgarci@telefonica.net"]

# You could put the mail in a file and use the '-infile' option
$curlHandle configure -readproc sendString

$curlHandle configure -verbose 1

$curlHandle perform

$curlHandle cleanup







