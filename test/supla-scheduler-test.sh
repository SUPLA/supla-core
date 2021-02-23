#!/bin/sh

set -e

[ -d ../test ] && cd ../
. ./test/common

cd supla-scheduler/Test 
make clean && make all 

vg_verify "./supla-scheduler"

echo OK 
exit 0
