#!/bin/bash

OMMIT=0
THREAD=0
PREVIOUS=""
LASTPRINTED=""

function print_line() {
   OMMIT=$1
   PREVIOUS=$2

   if [[ $PREVIOUS =~ ^\[New\ LWP ]]; then
      PREVIOUS=""
   fi

   if [ $OMMIT -eq 0 ]; then
     if ! [ "$PREVIOUS" = "$LASTPRINTED" ]; then
       echo $PREVIOUS
     fi
     LASTPRINTED=$PREVIOUS
   fi
}

while read -r line; do

  if [ $OMMIT -gt 0 ] && ! [[ "$line" =~ ^# ]]; then
    OMMIT=$((OMMIT-1))
  fi

  if [ $THREAD -eq 1 ]; then
    THREAD=0
    if [[ "$line" =~ ^#0 ]] && [[ "$line" =~ in\ epoll_wait ]]; then
       OMMIT=2
    fi
  fi

  if [ $OMMIT -eq 0 ]; then
    if [[ "$line" =~ ^Thread\ [0-9]+ ]]; then
      THREAD=1
    fi
  fi

  print_line $OMMIT "$PREVIOUS"
  PREVIOUS=$line

done < $1

print_line $OMMIT "$PREVIOUS" "$line"
