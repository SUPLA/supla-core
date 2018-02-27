#!/bin/sh

cpplint --filter=-build/include_subdir,-build/header_guard,-readability/casting,-runtime/int $1 && exit 0
exit 1
