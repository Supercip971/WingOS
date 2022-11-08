#!/bin/sh
set -e

qemu-system-x86_64 -m 2G -hda $BUILD_DIR/disk.hdd  \
    -device pvpanic  \
    -enable-kvm -no-reboot -no-shutdown \
    -d int,cpu_reset -enable-kvm -serial mon:stdio \