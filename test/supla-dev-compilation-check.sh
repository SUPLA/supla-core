#!/bin/sh

set -e

[ -d ../test ] && cd ../

cd supla-dev/Release 
make clean && make -j8
cd ../Debug 
make clean && make -j8

echo OK 
exit 0
