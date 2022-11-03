#!/bin/sh

PROJ=~/CProjects
/usr/local/bin/docker run -v "$PROJ":"$PROJ" devel/supla-core python /usr/local/bin/cpplint $@
