#!/bin/sh

set -e

[ -d ../test ] && cd ../
. ./test/common

DBHOST=db


mkdir -p /etc/supla-server

cd supla-server/Test 
make clean && make -j8 all 

cd ../../
db_init
cd supla-server/Test

vg_verify "./supla-server --sqldir ../../sql --gtest_shuffle --gtest_filter='-*DeathTest*'"
vg_verify "./supla-server --sqldir ../../sql --gtest_shuffle --gtest_filter='*DeathTest*'" --no-leak-check

echo OK 
exit 0
