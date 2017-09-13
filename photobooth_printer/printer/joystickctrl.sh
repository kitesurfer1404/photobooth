#!/bin/bash

trap "echo Exited!; exit;" SIGINT SIGTERM

#joy2key -rcfile joy2keyrc >>/dev/null &
joy2key -dev /dev/input/js0 -terminal -thresh 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -buttons a b c d >>/dev/null &

while true
do
  read -n 1 char
  xdotool type $char
done
