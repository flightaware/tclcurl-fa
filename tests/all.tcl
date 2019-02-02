#!/usr/bin/env tclsh

package require tcltest

::tcltest::configure -testdir [file dirname [file normalize [info script]]]

# The following will be upleveled and run whenever a test calls
# ::tcltest::loadTestedCommands
::tcltest::configure -load {

	namespace import ::tcltest::*
}

::tcltest::skipFiles [list]

# Allow command line arguments to be passed to the configure command
# This supports only running a single test or a single test file
::tcltest::configure {*}$argv

::tcltest::runAllTests
