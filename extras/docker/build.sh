#!/bin/sh

docker build -t "devel/supla-core-debian13" -f Dockerfile.debian13 .
docker build -t "devel/supla-core" .
