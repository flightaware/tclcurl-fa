#!/usr/bin/env tclsh

package require tcltest

::tcltest::configure -testdir [file dirname [file normalize [info script]]]

# The following will be upleveled and run whenever a test calls
# ::tcltest::loadTestedCommands
::tcltest::configure -load {

	namespace import ::tcltest::*
}

::tcltest::skipFiles [list]


# Hook to determine if any of the tests failed. Then we can exit with
# proper exit code: 0=all passed, 1=one or more failed
proc tcltest::cleanupTestsHook {} {
        variable numTests
        set ::exitCode [expr {$numTests(Failed) > 0}]
}



# Allow command line arguments to be passed to the configure command
# This supports only running a single test or a single test file
::tcltest::configure {*}$argv

::tcltest::runAllTests

if {$exitCode == 1} {
        puts "====== FAIL ====="
        exit $exitCode
} else {
        puts "====== SUCCESS ====="
}

