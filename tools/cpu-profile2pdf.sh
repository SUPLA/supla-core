#!/bin/sh

google-pprof --pdf $1 $1.prof > $1.pdf
