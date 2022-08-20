#!/bin/bash

set -e

if ! [ -e ./tools/lint.sh ]; then
  echo Run this script from the supla-core directory.
  echo "./tools/lint.sh"
  exit 1
fi

cpplint ./supla-server/src/* 
cpplint --filter=-build/include_what_you_use ./supla-server/src/actions/* 
cpplint ./supla-server/src/amazon/* 
cpplint ./supla-server/src/asynctask/* 
cpplint ./supla-server/src/channeljsonconfig/* 
cpplint ./supla-server/src/client/* 
cpplint ./supla-server/src/conn/* 
cpplint ./supla-server/src/device/* 
cpplint ./supla-server/src/device/call_handler* 
cpplint ./supla-server/src/distributedobjects/* 
cpplint ./supla-server/src/google/* 
cpplint ./supla-server/src/http/* 
cpplint ./supla-server/src/ipc/* 
cpplint --exclude=./supla-server/src/mqtt/mqtt.c --exclude=./supla-server/src/mqtt/mqtt.h ./supla-server/src/mqtt/*
cpplint ./supla-server/src/scene/* 
cpplint ./supla-server/src/srpc/* 
cpplint ./supla-server/src/user/* 
cpplint ./supla-server/src/webhook/* 
cpplint --filter=-build/include ./supla-server/src/test/* 
cpplint --filter=-build/include ./supla-server/src/test/actions/* 
cpplint --filter=-build/include ./supla-server/src/test/alexa/* 
cpplint --filter=-build/include ./supla-server/src/test/asynctask/* 
cpplint --filter=-build/include ./supla-server/src/test/channeljsonconfig/* 
cpplint --filter=-build/include ./supla-server/src/test/client/* 
cpplint --filter=-build/include ./supla-server/src/test/conn/* 
cpplint --filter=-build/include ./supla-server/src/test/google/* 
cpplint --filter=-build/include ./supla-server/src/test/http/* 
cpplint --filter=-build/include ./supla-server/src/test/mqtt/* 
cpplint --filter=-build/include ./supla-server/src/test/webhook/* 
cpplint --filter=-build/include ./supla-server/src/test/doubles/* 
cpplint --filter=-build/include ./supla-server/src/test/doubles/actions/*
cpplint --filter=-build/include ./supla-server/src/test/doubles/asynctask/*
cpplint --filter=-build/include ./supla-server/src/test/doubles/channeljsonconfig/*
cpplint --filter=-build/include ./supla-server/src/test/doubles/conn/*
cpplint --filter=-build/include ./supla-server/src/test/doubles/device/*
cpplint --filter=-build/include ./supla-server/src/test/doubles/distributedobjects/* 
cpplint --filter=-build/include ./supla-server/src/test/doubles/mqtt/*
cpplint --filter=-build/include ./supla-server/src/test/doubles/scene/*
cpplint --filter=-build/include ./supla-server/src/test/doubles/integration/asynctask/*
cpplint --filter=-build/include ./supla-server/src/test/doubles/integration/mqtt/*
cpplint --filter=-build/include ./supla-server/src/test/doubles/ipc/*
cpplint --filter=-build/include ./supla-server/src/test/actions/* 
cpplint --filter=-build/include ./supla-server/src/test/actions/* 
cpplint --filter=-build/include ./supla-server/src/test/asynctask/* 
cpplint --filter=-build/include ./supla-server/src/test/distributedobjects/* 
cpplint --filter=-build/include ./supla-server/src/test/mqtt/* 
cpplint --filter=-build/include ./supla-server/src/test/integration/* 
cpplint --filter=-build/include ./supla-server/src/test/ipc/* 
cpplint --filter=-build/include ./supla-server/src/test/integration/client/* 
cpplint --filter=-build/include ./supla-server/src/test/integration/mqtt/* 
cpplint --filter=-build/include ./supla-server/src/test/integration/scene/* 
cpplint ./supla-scheduler/src/* 
cpplint --filter=-build/include ./supla-scheduler/src/test/* 
cpplint --filter=-build/include ./supla-scheduler/src/test/doubles/* 
cpplint ./supla-dev/src/* 
cpplint ./supla-client/src/* 
cpplint ./supla-console-client/src/* 
cpplint ./supla-console-client/src/supla-client-lib/* 
cpplint --filter=-build/include ./supla-console-client/src/test/* 
cpplint --filter=-build/include ./supla-console-client/src/test/integration/* 
cpplint ./supla-afl/src/* 
echo OK
