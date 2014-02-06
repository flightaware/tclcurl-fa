package require TclCurl


curl::transfer -url 127.0.0.1/cgi-bin/post1.tcl -verbose 1 -post 1                   \
        -httppost [list name "firstName" contents "Andres" contenttype "text/plain" contentheader [list "adios: goodbye"]]                                       \
        -httppost [list name "lastName"  contents "Garcia"]                          \
        -httppost [list name "file"      file     "httpPost.tcl" file "basico.tcl" contenttype text/plain filename "c:\\basico.tcl"]                            \
        -httppost [list name "AnotherFile" filecontent "httpBufferPost.tcl"]         \
        -httppost [list name "submit"    contents "send"] -verbose 1





