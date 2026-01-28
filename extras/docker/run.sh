#!/bin/bash


mkdir -p data/mysql
mkdir -p data/postgresql

docker compose up -d 

if [ "$1" == "debian11" ]; then
  docker exec -it supla-core-devel-supla-core-debian11 /bin/bash
else
  docker exec -it supla-core-devel-supla-core /bin/bash
fi
