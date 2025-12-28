#!/bin/bash


mkdir -p data/mysql
mkdir -p data/postgresql

docker compose up -d 

if [ "$1" == "debian13" ]; then
  docker exec -it supla-core-devel-supla-core-debian13 /bin/bash
else
  docker exec -it supla-core-devel-supla-core /bin/bash
fi
