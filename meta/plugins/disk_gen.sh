#!/bin/sh
set -e

echo "==========================="
echo "This script is used to generate disk images for WingOS. Please run it under the python script"
echo "Due to partitioning and mounting, the build system may ask you to enter root privileges. Sorry for that"
echo "==========================="


DISK="./.cutekit/disk.hdd"
BOOT_MNT=""
MAIN_MNT=""
LOOP_DISK=""

cleanup() {
    if [ -n "$BOOT_MNT" ] && [ -d "$BOOT_MNT" ]; then
        sudo umount "$BOOT_MNT" 2>/dev/null || true
        rm -rf "$BOOT_MNT" 2>/dev/null || true
    fi
    if [ -n "$MAIN_MNT" ] && [ -d "$MAIN_MNT" ]; then
        sudo umount "$MAIN_MNT" 2>/dev/null || true
        rm -rf "$MAIN_MNT" 2>/dev/null || true
    fi
    if [ -n "$LOOP_DISK" ]; then
        sudo losetup -d "$LOOP_DISK" 2>/dev/null || true
    fi
}
trap cleanup EXIT INT TERM

rm -f "$DISK"
dd if=/dev/zero of="$DISK" bs=1M count=256

# CREATING PARTITION 1 (boot)
sgdisk "$DISK" -g -n 1:0:+100M -t 1:EF00 -c 1:"EFI System Partition"
if [ $? -ne 0 ]; then
    echo "Failed to create partition table on $DISK"
    exit 1
fi

# CREATING PARTITION 2 (system)
sgdisk "$DISK" -g -n 2:0:0 -t 2:8300 -c 2:"Linux Filesystem"

# CREATING LOOP DISK with losetup
LOOP_DISK="$(sudo losetup -Pf --show "$DISK")"
echo "using loop disk $LOOP_DISK"

if [ -z "$LOOP_DISK" ]; then
    echo "Error: Failed to create loop device for $DISK"
    exit 1
fi

# Wait for the kernel to recognize the partitions
echo "waiting for kernel to recognize partitions..."
sudo partprobe "$LOOP_DISK" 2>/dev/null || true
sudo udevadm settle 2>/dev/null || true
sync
echo "OK"

BOOT_DISK_PART="${LOOP_DISK}p1"
MAIN_DISK_PART="${LOOP_DISK}p2"

# Wait up to 10 seconds for partition devices to appear
for i in $(seq 1 10); do
    if [ -b "$BOOT_DISK_PART" ] && [ -b "$MAIN_DISK_PART" ]; then
        echo "Partition devices are ready: $BOOT_DISK_PART and $MAIN_DISK_PART"
        break
    fi
    echo "Waiting for partition devices to appear... (attempt $i/10)"
    sleep 1
done

# Final check that devices exist
if [ ! -b "$BOOT_DISK_PART" ]; then
    echo "Error: Boot partition device $BOOT_DISK_PART not found after waiting"
    exit 1
fi

if [ ! -b "$MAIN_DISK_PART" ]; then
    echo "Error: Main partition device $MAIN_DISK_PART not found after waiting"
    exit 1
fi

# CREATING BOOT PARTITION
echo "Requiring root privileges to format the disk: $BOOT_DISK_PART"
echo "Command running: sudo mkfs.fat $BOOT_DISK_PART"
sudo -E mkfs.fat "$BOOT_DISK_PART"
sync

# Mount boot partition
BOOT_MNT="$(mktemp -d)"
echo "Mounting boot partition to $BOOT_MNT"
sudo mount "$BOOT_DISK_PART" "$BOOT_MNT"

echo "Copying files to boot partition..."
sudo cp -r .cutekit/wingos-boot/* "$BOOT_MNT/"
sync

echo "Unmounting boot partition $BOOT_DISK_PART"
sudo umount "$BOOT_MNT"
rm -rf "$BOOT_MNT"
BOOT_MNT=""
sync

# CREATING SYSTEM PARTITION
echo "Requiring root privileges to format the disk: $MAIN_DISK_PART"
echo "Command running: sudo mke2fs -F $MAIN_DISK_PART"
sudo mke2fs -F "$MAIN_DISK_PART"
sync

# Mount system partition
MAIN_MNT="$(mktemp -d)"
echo "Mounting system partition to $MAIN_MNT"
sudo mount "$MAIN_DISK_PART" "$MAIN_MNT"
sudo chmod 777 "$MAIN_MNT"

echo "Copying files to system partition..."
sudo cp -r .cutekit/wingos-disk/* "$MAIN_MNT/"
sudo chmod -R 777 "$MAIN_MNT"
sync

echo "Unmounting system partition $MAIN_DISK_PART"
sudo umount "$MAIN_MNT"
rm -rf "$MAIN_MNT"
MAIN_MNT=""
sync

# Detach loop device
sudo losetup -d "$LOOP_DISK"
LOOP_DISK=""

echo "Successfully created disk image at $DISK"
exit 0
