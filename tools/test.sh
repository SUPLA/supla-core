#!/bin/sh

set -e

[ -d /etc/supla-server ] || sudo mkdir /etc/supla-server
[ -e /etc/supla-server/supla-test.cfg ] || sudo touch /etc/supla-server/supla-test.cfg

cd supla-server/Test 
make clean && make all 
./supla-server 

../../tools/valgrind-full.sh ./supla-server > /dev/null 2> vg-test.log

(cat ./vg-test.log | grep "All heap blocks were freed -- no leaks are possible" ) \
|| (cat ./vg-test.log && echo "Memory leak error!" && rm ./vg-test.log && exit 1)

rm ./vg-test.log
cd ../Release 
make clean && make all 
cd ../../supla-scheduler/Test 
make clean && make all 
./supla-scheduler 
cd ../Release 
make clean && make all 
cd ../../supla-dev/Release 
make clean && make 
cd ../../supla-console-client/Debug 
make clean && make 

echo OK 
exit 0
