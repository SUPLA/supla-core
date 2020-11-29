#!/bin/bash

set -e

if ! [ -e ./tools/lint.sh ]; then
  echo Run this script from the supla-core directory.
  echo "./tools/lint.sh"
  exit 1
fi

cpplint ./supla-server/src/* 
cpplint ./supla-server/src/amazon/* 
cpplint ./supla-server/src/client/* 
cpplint ./supla-server/src/device/* 
cpplint ./supla-server/src/google/* 
cpplint ./supla-server/src/http/* 
cpplint ./supla-server/src/user/* 
cpplint --exclude=./supla-server/src/mqtt/mqtt.c --exclude=./supla-server/src/mqtt/mqtt.h ./supla-server/src/mqtt/*
cpplint ./supla-server/src/test/* 
cpplint ./supla-server/src/test/alexa/* 
cpplint ./supla-server/src/test/google/* 
cpplint ./supla-server/src/test/integration/* 
cpplint ./supla-server/src/test/integration/mqtt/* 
cpplint ./supla-server/src/test/mqtt/* 
cpplint ./supla-server/src/test/webhook/* 
cpplint ./supla-scheduler/src/* 
cpplint ./supla-scheduler/src/test/* 
cpplint ./supla-dev/src/* 
cpplint ./supla-client/src/* 
cpplint ./supla-console-client/src/* 
cpplint ./supla-console-client/src/supla-client-lib/* 
cpplint ./supla-console-client/src/test/* 
cpplint ./supla-console-client/src/test/integration/* 
cpplint ./supla-afl/src/* 
echo OK
