#!/bin/sh

valgrind --tool=memcheck --leak-check=full --undef-value-errors=no $1
