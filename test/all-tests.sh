#!/bin/bash

set -e

[ -d ./test ] && cd ./test

./supla-server-test.sh
./supla-scheduler-test.sh
./supla-dev-test.sh
./supla-console-client-test.sh
