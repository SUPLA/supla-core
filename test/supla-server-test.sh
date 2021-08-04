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

cat > /etc/supla-server/supla-test.cfg <<- ENDOFCFG
[NET]
cert=/etc/supla-server/cert.crt
private_key=/etc/supla-server/private.key

[MySQL]
host=$DBHOST
port=3306
database=supla_test
user=supla

[MQTT-BROKER]
enabled=1
host=192.168.111.30
port=1883
ssl=0
username=supla-server
password=alkdmalsdaskdn
client_id=NunYnx
ENDOFCFG

vg_verify "./supla-server --sqldir ../../sql"

echo OK 
exit 0
