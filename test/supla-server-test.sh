#!/bin/sh

set -e

[ -d ../test ] && cd ../
. ./test/common

mkdir -p /etc/supla-server

cd supla-server/Test 
make clean && make all 

[NET]
cert=/etc/supla-server/cert.crt
private_key=/etc/supla-server/private.key

cat > /etc/supla-server/supla-test.cfg <<- ENDOFCFG
[MySQL]
host=db
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
prefix=%email%
client_id=NunYnx
ENDOFCFG

vg_verify "./supla-server"

cd ../Release 
make clean && make all 
cd ../Debug 
make clean && make all 

echo OK 
exit 0
