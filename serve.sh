#!/bin/bash
kill $(ps aux | grep '[w]eb-server' | awk '{print $2}')
make clean
clear
make
./web-server &
sleep 0.3
./web-client http://localhost:4000/index.html http://localhost:4000/a.txt http://localhost:4000/b.txt
# wget http://localhost:4000/a.txt
# curl http://localhost:4000/a.txt --http1.0 -O
# curl http://localhost:4000/b.txt --http1.0 -O
