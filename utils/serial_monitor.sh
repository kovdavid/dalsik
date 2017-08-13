#!/bin/sh

TARGET=${1:-/dev/ttyACM0}

while read -r line < $TARGET; do
    echo $line
    echo "$(date '+%FT%T')======"
done
