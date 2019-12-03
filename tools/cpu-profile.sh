#!/bin/sh

LD_PRELOAD=/usr/lib/libprofiler.so CPUPROFILE=$1.prof CPUPROFILE_FREQUENCY=100000 $1
