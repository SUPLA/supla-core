#!/bin/sh

[ -d /etc/supla-server ] || sudo mkdir /etc/supla-server
[ -e /etc/supla-server/supla-test.cfg ] || sudo touch /etc/supla-server/supla-test.cfg

cd supla-server/Test && \
make clean && make all && \
./supla-server && \
cd ../Release && \
make clean && make all && \
cd ../../supla-scheduler/Test && \
make clean && make all && \
./supla-scheduler && \
cd ../Release && \
make clean && make all && \
cd ../../supla-dev/Release && \
make clean && make && \
cd ../../supla-console-client/Debug && \
make clean && make && \
echo OK && \
exit 0

echo FAILED
exit 1


