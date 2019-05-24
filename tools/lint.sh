#!/bin/sh

cpplint ./supla-server/src/* && \
cpplint ./supla-server/src/amazon/* && \
cpplint ./supla-server/src/client/* && \
cpplint ./supla-server/src/device/* && \
cpplint ./supla-server/src/google/* && \
cpplint ./supla-server/src/http/* && \
cpplint ./supla-server/src/user/* && \
cpplint ./supla-server/src/test/* && \
cpplint ./supla-scheduler/src/* && \
cpplint ./supla-scheduler/src/test/* && \
cpplint ./supla-dev/src/* && \
cpplint ./supla-client/src/* && \
cpplint ./supla-console-client/src/* && \
cpplint ./supla-console-client/src/supla-client-lib/* && \
cpplint ./supla-afl/src/* && \
echo OK && \
exit 0

echo FAILED
exit 1
