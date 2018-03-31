#!/bin/sh

cppcheck --version
cppcheck --inline-suppr --error-exitcode=1 -q -i ./supla-server/src/test ./supla-server/src/*  && \
cppcheck --inline-suppr --error-exitcode=1 -q -i ./supla-scheduler/src/test ./supla-scheduler/src/* && \
exit 0

exit 1
