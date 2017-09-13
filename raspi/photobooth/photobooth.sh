#!/bin/bash

echo "You can stop the photobooth-script with Ctrl+C"
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

python "$BASEDIR/server.py" 8000 &      # start slideshow server
python "$BASEDIR/shutdown.py" &         # start shutdown script

# Try over and over. 
# Camera might not be connected yet or battery died.
# See README for details on the capturetarget
while [ 1 ]
do
  gphoto2 --set-config-index /main/settings/capturetarget=1
  RESULT=$?
  if [ $RESULT -eq 0 ]; then
    gphoto2 --wait-event-and-download --keep --hook-script="$BASEDIR/postprocessing.sh"
  fi
  sleep 10
done
