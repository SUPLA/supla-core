#!/bin/bash

set -e

PROJ=~/CProjects
DATADIR=~/Database/mariadb/supla-core-devel

[ -e $PROJ ] || mkdir -p $PROJ
[ -e $DATADIR ] || mkdir -p $DATADIR

docker network ls |grep supla-core-devel-net > /dev/null ||\
 docker network create --gateway 192.168.111.1 --subnet 192.168.111.0/24 supla-core-devel-net

docker start supla-core-devel-mariadb 2> /dev/null > /dev/null || \
docker run -v "$DATADIR":/var/lib/mysql --rm --network supla-core-devel-net --ip 192.168.111.2\
 --name supla-core-devel-mariadb\
 -e MYSQL_ROOT_PASSWORD=root -d mariadb\
 --character-set-server=utf8mb4\
 --collation-server=utf8mb4_unicode_ci

cd "$PROJ"
[ ! -e ./supla-core ] && git clone https://github.com/SUPLA/supla-core
docker run -p 2015:2015/tcp -p 2016:2016/tcp -v "$PROJ":/CProjects --network supla-core-devel-net\
 --add-host=db:192.168.111.2 --ip 192.168.111.20 -it devel/supla-core:latest /bin/bash
