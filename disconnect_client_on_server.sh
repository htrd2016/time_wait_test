./SendData 127.0.0.1 3001 "close:10" && sleep 3 && netstat -n|awk '/^tcp/{++S[$NF]}END {for(a in S) print a, S[a]}'
