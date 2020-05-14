#!/bin/sh

set -e

[ -d ../test ] && cd ../
. ./test/common

DBHOST=db

if ! (echo "" | mysql -u supla -h $DBHOST) 2> /dev/null; then
  if ! mysql -u root -h $DBHOST < sql/CreateSqlUsersForTestPurposes.sql 2> /dev/null; then
    if ! mysql -u root -proot -h $DBHOST < sql/CreateSqlUsersForTestPurposes.sql 2> /dev/null; then
      echo "Enter mariadb root password.";
      mysql -u root -h $DBHOST -p < sql/CreateSqlUsersForTestPurposes.sql 
    fi
  fi
fi

mysql -u supla -h $DBHOST < sql/RecreateTestDatabase.sql
mysql -u supla -h $DBHOST supla_test < sql/TestDatabaseStructureAndData.sql

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
  cd ./supla-server/Release
  if ! [ -e ./supla-server ]; then
     make clean && make all
  fi
  ./supla-server -c /etc/supla-server/supla-test.cfg -d
  cd ../../
fi

sleep 2
[ -e /var/log/syslog ] && grep "Incorrect database version!" /var/log/syslog && exit 1

cd supla-console-client/Test
make clean && make all

vg_verify "./supla-console-client -sqldir ../../sql"

cd ../Debug 
make clean && make 
cd ../Release
make clean && make 

echo OK 
exit 0
