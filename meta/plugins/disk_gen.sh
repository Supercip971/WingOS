#!/bin/sh 

set -e 
echo "This script is used to generate disk images for WingOS. Please run it under the python script"
echo "Due to partitionning and mounting, the build system may ask you to enter root privileges. Sorry for that"

DISK="./.cutekit/disk.hdd"
MNT_PATH="./mnt"

rm -rf $DISK
dd if=/dev/zero of=$DISK bs=1M count=512

# CREATING PARTITION 1 (boot)
sgdisk $DISK -g -n 1:0:+100M -t 1:EF00 -c 1:"EFI System Partition" 
if [ $? -ne 0 ]; then
    echo "Failed to create partition table on $DISK"
    exit 1
fi

# CREATING PARTITION 2 (system)
sgdisk $DISK -g -n 2:0:0 -t 2:8300 -c 2:"Linux Filesystem"


# CREATING LOOP DISK with udisksctl
LOOP_DISK_S="`udisksctl loop-setup -f ./.cutekit/disk.hdd`"
LOOP_DISK_ST=${LOOP_DISK_S#*as }
LOOP_DISK=${LOOP_DISK_ST%*.}
echo "using loop disk $LOOP_DISK"


# CREATING BOOT PARTITION
BOOT_DISK_PART="${LOOP_DISK}p1"
echo "Requiring root privileges to format the disk: $BOOT_DISK_PART"
echo "Command running: sudo mkfs.fat $BOOT_DISK_PART"
sudo mkfs.fat $BOOT_DISK_PART

BOOT_DISK_PATH_S="`udisksctl mount -b $BOOT_DISK_PART --no-user-interaction`"

BOOT_DISK_PATH=${BOOT_DISK_PATH_S#*at }

echo "Mounting boot partition at $BOOT_DISK_PATH"
cp -r .cutekit/wingos-boot/* $BOOT_DISK_PATH

udisksctl unmount -b $BOOT_DISK_PART
if [ $? -ne 0 ]; then
    echo "Failed to unmount boot partition $BOOT_DISK_PART"
    exit 1
fi

echo "Successfully unmounted boot partition $BOOT_DISK_PART"


#CREATING SYSTEM PARTITION
MAIN_DISK_PART="${LOOP_DISK}p2"
echo "Requiring root privileges to format the disk: $MAIN_DISK_PART"
echo "Command running: sudo mke2fs $MAIN_DISK_PART"
sudo mke2fs -E root_perms=777  $MAIN_DISK_PART
MAIN_DISK_PATH_S="`udisksctl mount -b $MAIN_DISK_PART --no-user-interaction`"
MAIN_DISK_PATH=${MAIN_DISK_PATH_S#*at }
echo "Mounting system partition at $MAIN_DISK_PATH"
mkdir -p $MAIN_DISK_PATH
cp -r .cutekit/wingos-disk/* $MAIN_DISK_PATH
echo "Unmounting system partition $MAIN_DISK_PART"
udisksctl unmount -b $MAIN_DISK_PART
if [ $? -ne 0 ]; then
    echo "Failed to unmount system partition $MAIN_DISK_PART"
    exit 1
fi
echo "Successfully unmounted system partition $MAIN_DISK_PART"

echo "Successfully created disk image at $DISK"
sync 

udisksctl loop-delete -b $LOOP_DISK

exit 0