#!/bin/bash

PHOTOBOOTH_SERVER='http://10.0.0.1:8000'
IMAGE_PATH='/images/processed/'

lockdir=/tmp/photobooth_print.lock
tmpfile=`mktemp`
if mkdir "$lockdir"
then
    echo >&2 "$0 : successfully acquired lock"

    # Remove lockdir when the script finishes, or when it receives a signal
    trap 'rm -rf "$lockdir"' 0    # remove directory when script finishes
    trap 'rm $tmpfile' 0

    wget "$PHOTOBOOTH_SERVER""$IMAGE_PATH""$1" -O "$tmpfile"
    echo "$0 : printing $tmpfile"

    #lp -d selphy "$tmpfile" &
    #lpq -P selphy +5

    ./selphy -printer_ip 10.0.0.2 "$tmpfile"
    echo "selphy done."

    echo "$0 : Cleaning up."
    rm -rf "$lockdir"
    echo "$0 : done. Bye."
else
    echo >&2 "cannot acquire lock, giving up on $lockdir"
    exit 0
fi
