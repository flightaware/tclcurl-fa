package require TclCurl

# This is one contrived example, but it works.

proc writeToFile {readData} {
    puts "writeToFile called [incr ::i]"
    puts -nonewline $::inFile $readData

    return
}

set i 0

set inFile [open "cosa.tar" w+]
fconfigure $inFile -translation binary

curl::transfer -url "127.0.0.1/~andres/cosa&co.tar" \
        -writeproc writeToFile -buffersize 250

close $inFile




