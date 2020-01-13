#!/bin/bash

while true ; do
./supla-console-client -h devel-cloud.supla.org -email p.zygmunt@acsoftware.pl | grep Registered && touch reg
done
