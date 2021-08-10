#!/bin/sh

LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libprofiler.so CPUPROFILE=$1.prof CPUPROFILE_FREQUENCY=100000 $1
