package require TclCurl

curl::transfer -url "127.0.0.1" -verbose 1 -nobody 1 -header 1     \
        -http200aliases [list "yummy/4.5 200 OK" "great/1.3 350 WRONG"]

        






