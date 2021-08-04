#!/bin/sh

set -e

[ -d ../test ] && cd ../

cd supla-console-client/Debug 
make clean && make -j8
cd ../Release
make clean && make -j8

echo OK 
exit 0
