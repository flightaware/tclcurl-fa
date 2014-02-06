package require TclCurl

# This is one contrived example, but it works.

proc writeToFile {readData} {
    puts "writeToFile called [incr ::i]"
    puts -nonewline $::outFile $readData

    return
}

set i 0

set outFile [open "cosa.tar" w+]
fconfigure $outFile -translation binary

curl::transfer -url "127.0.0.1/~andres/cosa&co.tar" -writeproc writeToFile

close $outFile




