# Contributed by Jos Decoster

set curlHandle [curl::init]

$curlHandle configure                                        \
        -verbose 1 -failonerror 1 -errorbuffer ::errorBuffer \
        -url $::url -file $::file                            \
        -proxy $::proxy_host -proxyport $::proxy_port        \
        -proxyauth ntlm                                      \
        -proxyuserpwd $::proxy_user:$::proxy_password

if { [catch {$curlHandle perform} r] } {
    return -code error "$r $::errorBuffer"
}

$curlHandle cleanup
