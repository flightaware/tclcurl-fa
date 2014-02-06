package require TclCurl


curl::transfer -url ftp://andres:ciclope4@127.0.01/cosa.tcl                 \
        -infilesize [file size writeProc.tcl] -infile writeProc.tcl -upload 1 \
        -verbose 1


