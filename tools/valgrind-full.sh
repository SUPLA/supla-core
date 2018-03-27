#!/bin/sh

valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all --track-origins=yes $1 
