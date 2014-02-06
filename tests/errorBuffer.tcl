package require TclCurl

set curlHandle [curl::init]
$curlHandle configure -url "Shire.Asturias.com" -errorbuffer errorMsg

if {[catch {$curlHandle perform}]} {
	puts "The error message: $errorMsg"
}

$curlHandle configure -url "Shire.Asturias.com" -errorbuffer error(msg)

if {[catch {$curlHandle perform}]} {
	puts "The error message: $error(msg)"
}

$curlHandle cleanup

unset error
catch {curl::transfer -url "Shire.Asturias.com" -errorbuffer error(msg)}

puts "Error: $error(msg)"


