#!/bin/sh

set -e

[ -d ../test ] && cd ../
. ./test/common

DBHOST=db

db_init

mkdir -p /etc/supla-server
cp ./test/cert/cert.crt /etc/supla-server/
cp ./test/cert/private.key /etc/supla-server/

cat > /etc/supla-server/supla-test.cfg <<- ENDOFCFG
[NET]
cert=/etc/supla-server/cert.crt
private_key=/etc/supla-server/private.key

[MySQL]
host=$DBHOST
port=3306
database=supla_test
user=supla
ENDOFCFG

if ! ps ax|grep supla-server|grep -v grep > /dev/null; then
  cd ./supla-server/Debug
  if ! [ -e ./supla-server ]; then
     make clean && make -j8 all
  fi
  ./supla-server -c /etc/supla-server/supla-test.cfg -d
  cd ../../
fi

sleep 2
[ -e /var/log/syslog ] && grep "Incorrect database version!" /var/log/syslog && exit 1

if ! ps ax|grep supla-server|grep -v grep > /dev/null; then
grep supla-server /var/log/syslog
echo "supla-server did not start!"
exit 1
fi

cd supla-console-client/Test
make clean && make -j8 all

vg_verify "./supla-console-client --sqldir ../../sql"

echo OK 
exit 0
