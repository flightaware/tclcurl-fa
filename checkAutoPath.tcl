# This file checks that the directory where TclCurl is going to be
# installed is actually in the auto_path variable, if it isn't it 
# modifies 'init.tcl'.

set tclCurlDir  [lindex $argv 0]
set initTclDir  [lindex $argv 1]
set initFile    [file join $tcl_library init.tcl]
if {[file writable $initFile]==0} exit


if {[lsearch $auto_path $tclCurlDir]==-1} {
    set handle [open "$initFile" a]
    puts $handle "\nlappend auto_path $tclCurlDir\n"
}

exit

