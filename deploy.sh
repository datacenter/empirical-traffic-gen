#!/bin/bash

## Compile binaries
make clean
make
cp src/client/client .
cp src/server/server .

declare -a server=(172.21.158.31 172.21.158.45);
declare -a user=(mohammad root);

for (( i=0; i<${#server[@]}; i++ ));
do
    echo "****************************"
    echo "syncing to  ${server[$i]}..."
    rsync -pvz *CDF* ${user[$i]}\@${server[$i]}:~/traffic_gen
    rsync -pvz client ${user[$i]}\@${server[$i]}:~/traffic_gen
    rsync -pvz server ${user[$i]}\@${server[$i]}:~/traffic_gen
    rsync -pvz scripts/* ${user[$i]}\@${server[$i]}:~/traffic_gen     
done