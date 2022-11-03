#!/bin/bash

PROJ=~/CProjects
cat /dev/stdin | /usr/local/bin/docker run -i -v "$PROJ":"$PROJ" devel/supla-core /usr/bin/clang-format-4.0 $@ 
