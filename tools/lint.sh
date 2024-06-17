#!/bin/bash

set -e

if ! [ -e ./tools/lint.sh ]; then
  echo Run this script from the supla-core directory.
  echo "./tools/lint.sh"
  exit 1
fi

find ./supla-server/src -type f \
! -path ./supla-server/src/mqtt/mqtt.c \
! -path ./supla-server/src/mqtt/mqtt.h \
! -path "./supla-server/src/crypt_blowfish/*" \
! -path "./supla-server/src/date/date.h" \
! -path "./supla-server/src/date/tz.cpp" \
! -path "./supla-server/src/date/tz.h" \
! -path "./supla-server/src/date/tz_private.h" \
! -path "./supla-server/src/astro/SolarCalculator.cpp" \
! -path "./supla-server/src/astro/SolarCalculator.h" \
-exec cpplint {} +

find ./supla-scheduler/src -type f -exec cpplint {} +
find ./supla-dev/src -type f -exec cpplint {} +
find ./supla-console-client/src -type f -exec cpplint {} +
find ./supla-afl/src -type f -exec cpplint {} +

echo OK
