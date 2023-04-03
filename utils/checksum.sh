#!/bin/bash

if [ ! -f "$DALSIK_KEYMAP" ] ; then
    echo "Variable DALSIK_KEYMAP is not set"
    exit 1
fi

SOURCE_CHECKSUMS=$(find lib/ src/ \( -name '*.h' -or -name '*.cpp' \) -exec sha256sum {} \; | sort -k2)
KEYMAP_CHECKSUM=$(sha256sum $DALSIK_KEYMAP | awk '{ print $1 }')

FINAL_CHECKSUM=$(echo -e "$SOURCE_CHECKSUMS\n$KEYMAP_CHECKSUM" | sha256sum)

SHORT_CHECKSUM=${FINAL_CHECKSUM:0:7}

echo "$SHORT_CHECKSUM"
