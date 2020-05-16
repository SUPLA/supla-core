#!/bin/sh

set -e

[ -d ../test ] && cd ../
. ./test/common

mkdir -p /etc/supla-server
[ -e /etc/supla-server/supla-test.cfg ] || touch /etc/supla-server/supla-test.cfg

cd supla-server/Test 
make clean && make all 

vg_verify "./supla-server"

cd ../Release 
make clean && make all 
cd ../Debug 
make clean && make all 

echo OK 
exit 0
