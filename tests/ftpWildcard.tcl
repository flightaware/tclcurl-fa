package require TclCurl

proc FtpMatch {pattern filename} {

    puts "Pattern: $pattern - File: $filename"

    # For this example everything matches
    return 0
}

proc FtpCheck {remains} {
    global someVar

    # Lets forget about directories:
    if {($someVar(filetype) eq "directory") || ([regexp {^\.} $someVar(filename)])} {
        return 1
    }

    puts -nonewline "File to download $someVar(filename) ($someVar(size)B) (y/N): "
    flush stdout
    set line [string tolower [gets stdin]]
    if {$line eq y} {
        return 0
    }
    return  1
}

proc FtpSaveFile {readData} {
    global   outFile
    global   openedFile
    global   someVar

    if {$openedFile==0} {
        if {![file exists downloads]} {
            file mkdir downloads
        }
        set outFile [open "downloads/$someVar(filename)" w+]
        fconfigure $outFile -translation binary
    }

    puts -nonewline $outFile $readData

    return 0
}

proc FtpDone {} {
    global outFile
    global openedFile

    puts "Done\n"

    close $outFile
    set openedFile 0

    return 0
}

set openedFile 0
set curlHandle [curl::init]

$curlHandle configure -url ftp://sunsite.rediris.es/sites/metalab.unc.edu/ldp/*
$curlHandle configure -chunkbgnproc  FtpCheck
$curlHandle configure -chunkbgnvar   someVar
$curlHandle configure -chunkendproc  FtpDone
$curlHandle configure -writeproc     FtpSaveFile
$curlHandle configure -wildcardmatch 1
$curlHandle configure -fnmatchproc   FtpMatch

$curlHandle perform

$curlHandle cleanup










