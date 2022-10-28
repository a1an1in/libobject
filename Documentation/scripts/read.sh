#!/usr/bin/env bash

# to be executed on E/// TTRD machine to record CPRIC events
SERVER=10.152.89.149
WAIT_TIME=10
scp ./read_events.sh root@${SERVER}:/root/read_events.sh
base_addr=`ssh root@${SERVER} "plf fw sarek0 -b | grep 'FPGA base address:' | awk '{print $4}'"`
ssh root@${SERVER} \
"chmod +x ./read_events.sh && \
./read_events.sh -b=${base_addr} --restart && \
sleep ${WAIT_TIME} && \
./read_events.sh -b=${base_addr} --read > afry_shuo.log"
echo root | scp root@${SERVER}:/root/afry_shuo.log ./
