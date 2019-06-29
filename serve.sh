#!/bin/bash
kill $(ps aux | grep '[w]eb-server' | awk '{print $2}')
make clean
clear
make
./web-server &
./web-client http://localhost:4000/index.html http://localhost:4000/a.txt
