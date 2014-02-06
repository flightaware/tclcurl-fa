package require TclCurl

set buffer ""
if {[catch {curl::transfer -url ftp://127.0.0.1/Test/cosa.tcl \
        -userpwd "user:pwd" -verbose 1                        \
        -infile ftpUpload.tcl -upload 1  -errorbuffer buffer  \
        -quote [list "mkd Test"]                              \
        -postquote [list "rnfr cosa.tcl" "rnto script.tcl"]   \
      } buffer]} {
    puts "Error: $buffer"
} else {
    puts "Upload complete"
}












