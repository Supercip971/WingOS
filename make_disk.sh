#!/bin/bash

# inspired from https://github.com/ozkl/soso/blob/master/create-image.sh (from soso os) made by ozkl <3

DISK="./build/disk.hdd"
MNT_PATH="./mnt"
dd if=/dev/zero of=$DISK bs=250M count=1

fdisk $DISK << EOF
n
p
1


a
w
EOF

LOOP_DISK="`losetup -f`"
echo "using loop disk $LOOP_DISK"
sudo losetup $LOOP_DISK $DISK
LOOP_DISK_PARTIITION="`losetup -f`"
sudo losetup $LOOP_DISK_PARTIITION $DISK -o 1048576
sudo mke2fs $LOOP_DISK_PARTIITION
mkdir -p $MNT_PATH
sudo mount $LOOP_DISK_PARTIITION $MNT_PATH

mkdir $MNT_PATH/initfs
cp -r ./initfs/* $MNT_PATH/initfs/
cp ./limine.cfg $MNT_PATH/limine.cfg
cp ./limine/limine.sys $MNT_PATH/limine.sys
cp ./kernel.elf $MNT_PATH/kernel.elf
ls -la $MNT_PATH
ls -la $MNT_PATH/initfs
umount $MNT_PATH
losetup -d $LOOP_DISK_PARTIITION
losetup -d $LOOP_DISK
sync 

fdisk -l $DISK

limine/limine-install-linux-x86_64 $DISK
