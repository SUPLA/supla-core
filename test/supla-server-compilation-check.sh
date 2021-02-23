#!/bin/sh

set -e

[ -d ../test ] && cd ../

cd supla-server/Release 
make clean && make -j8 all 
cd ../Debug 
make clean && make -j8 all 

echo OK 
exit 0
