#!/bin/sh

set -e

[ -d ../test ] && cd ../
. ./test/common

cd supla-dev/Release 
make clean && make 
cd ../Debug 
make clean && make 

echo OK 
exit 0
