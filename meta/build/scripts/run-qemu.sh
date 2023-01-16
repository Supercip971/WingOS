#!/bin/sh
set -e

qemu-system-x86_64 -m 2G -hda $BUILD_DIR/disk.hdd  \
    -device pvpanic  \
    -no-shutdown \
    -d int  -serial mon:stdio \