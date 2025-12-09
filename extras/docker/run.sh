#!/bin/bash


mkdir -p data/mysql
mkdir -p data/postgresql
docker compose up -d
docker exec -it supla-core-devel-supla-core /bin/bash
