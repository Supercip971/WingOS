#!/bin/bash

# inspired from https://github.com/ozkl/soso/blob/master/create-image.sh (from soso os) made by ozkl <3

DISK="./build/disk.hdd"
dd if=/dev/zero of=$DISK bs=64M count=1

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
sudo mount $LOOP_DISK_PARTIITION /mnt

mkdir /mnt/init_fs
cp -r ./init_fs/* /mnt/init_fs/
cp ./limine.cfg /mnt/limine.cfg
cp ./kernel.elf /mnt/kernel.elf
ls -la /mnt
ls -la /mnt/init_fs
umount /mnt
losetup -d $LOOP_DISK_PARTIITION
losetup -d $LOOP_DISK
sync 

fdisk -l $DISK

limine/limine-install limine/limine.bin $DISK
