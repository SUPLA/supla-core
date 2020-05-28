#!/bin/sh

set -e

[ -d ../test ] && cd ../
. ./test/common

cd supla-scheduler/Test 
make clean && make all 

vg_verify "./supla-scheduler"

cd ../Release 
make clean && make all 
cd ../Debug
make clean && make all

echo OK 
exit 0
