#!/bin/bash

echo "You can stop the printer-script with Ctrl+C"
echo -n "Starting in "

for i in $(seq 5 -1 1)
do
    echo -n "$i "
    sleep 1
done

echo ""
echo "Starting services..."

SCRIPT=$(realpath $0)
BASEDIR=$(dirname $SCRIPT)

cd "$BASEDIR"
trap 'jobs -p | xargs kill' EXIT

python "$BASEDIR/server.py" 8080 &      # start webserver
unclutter -idle 0 &                     # hide cursor
firefox --url http://localhost:8080 &   # start firefox
xdotool search --sync --onlyvisible --class "Firefox" windowactivate key F11  # switch firefox to fullscreen
"$BASEDIR/joystickctrl.sh"              # start joystick ctrl

while [ 1 ]
do
  sleep 10
done
