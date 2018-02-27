#!/bin/sh

valgrind --tool=memcheck --leak-check=full --track-origins=yes --undef-value-errors=no $i
