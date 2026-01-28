#!/bin/sh

docker build -t "devel/supla-core-debian11" -f Dockerfile.debian11 .
docker build -t "devel/supla-core" .
