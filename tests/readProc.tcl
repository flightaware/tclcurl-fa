package require TclCurl

# As an example this is very contrived, but it works.

proc readFile {size} {

    set chunk [read $::inFile $size]

    return $chunk
}

set inFile [open "cosa.tar" r]
fconfigure $inFile -translation binary

curl::transfer -url "ftp://127.0.0.1/cosa.tar" -verbose 1 \
	-username user -password pass -readproc readFile -upload 1

close $inFile





