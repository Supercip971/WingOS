#!/bin/sh

echo "This script is used to generate disk images for WingOS. Please run it under the python script"
echo "Due to partitionning and mounting, the build system may ask you to enter root privileges. Sorry for that"

DISK="./.cutekit/disk.hdd"
MNT_PATH="./mnt"

rm -rf $DISK
dd if=/dev/zero of=$DISK bs=1M count=256

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

# Wait for the kernel to recognize the partitions
# This fixes the race condition where partition devices aren't ready yet
sudo partprobe $LOOP_DISK
sync

# Wait for partition device nodes to appear
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
    udisksctl loop-delete -b $LOOP_DISK
    exit 1
fi

if [ ! -b "$MAIN_DISK_PART" ]; then
    echo "Error: Main partition device $MAIN_DISK_PART not found after waiting"
    udisksctl loop-delete -b $LOOP_DISK
    exit 1
fi


# CREATING BOOT PARTITION
echo "Requiring root privileges to format the disk: $BOOT_DISK_PART"
echo "Command running: sudo mkfs.fat $BOOT_DISK_PART"
sudo mkfs.fat $BOOT_DISK_PART
if [ $? -ne 0 ]; then
    echo "Failed to format boot partition $BOOT_DISK_PART"
    udisksctl loop-delete -b $LOOP_DISK
    exit 1
fi
sync

# Try mounting boot partition with retries
BOOT_DISK_PATH=""
for i in $(seq 1 5); do
    echo "Attempting to mount boot partition... (attempt $i/5)"
    BOOT_DISK_PATH_S="`udisksctl mount -b $BOOT_DISK_PART 2>&1`"
    status=$?
    
    if [ $status -eq 0 ]; then
        BOOT_DISK_PATH=${BOOT_DISK_PATH_S#*at }
        BOOT_DISK_PATH=$(echo "$BOOT_DISK_PATH" | tr -d '.')
        echo "Successfully mounted boot partition at $BOOT_DISK_PATH"
        break
    fi
    
    # Check if already mounted - extract path between backtick and single quote
    # Format: "...is already mounted at \`/run/media/user/xxx'."
    case "$BOOT_DISK_PATH_S" in
        *AlreadyMounted*)
            BOOT_DISK_PATH="${BOOT_DISK_PATH_S#*\`}"
            BOOT_DISK_PATH="${BOOT_DISK_PATH%%\'*}"
            if [ -n "$BOOT_DISK_PATH" ] && [ -d "$BOOT_DISK_PATH" ]; then
                echo "Boot partition already mounted at $BOOT_DISK_PATH (using existing mount)"
                break
            fi
            ;;
    esac
    
    echo "Mount failed, retrying..."
    sleep 1
done

if [ -z "$BOOT_DISK_PATH" ]; then
    echo "Failed to mount boot partition after 5 attempts"
    udisksctl loop-delete -b $LOOP_DISK
    exit 1
fi

cp -r .cutekit/wingos-boot/* $BOOT_DISK_PATH
if [ $? -ne 0 ]; then
    echo "Failed to copy files to boot partition"
    udisksctl unmount -b $BOOT_DISK_PART
    udisksctl loop-delete -b $LOOP_DISK
    exit 1
fi
sync

echo "Unmounting boot partition $BOOT_DISK_PART"
udisksctl unmount -b $BOOT_DISK_PART
if [ $? -ne 0 ]; then
    echo "Failed to unmount boot partition $BOOT_DISK_PART"
    udisksctl loop-delete -b $LOOP_DISK
    exit 1
fi

echo "Successfully unmounted boot partition $BOOT_DISK_PART"
sync

#CREATING SYSTEM PARTITION
echo "Requiring root privileges to format the disk: $MAIN_DISK_PART"
echo "Command running: sudo mke2fs $MAIN_DISK_PART"
sudo mke2fs -E root_perms=777 $MAIN_DISK_PART
if [ $? -ne 0 ]; then
    echo "Failed to format system partition $MAIN_DISK_PART"
    udisksctl loop-delete -b $LOOP_DISK
    exit 1
fi
sync
sleep 0.5

# Try mounting system partition with retries
MAIN_DISK_PATH=""
for i in $(seq 1 5); do
    echo "Attempting to mount system partition... (attempt $i/5)"
    MAIN_DISK_PATH_S="$(udisksctl mount -b "$MAIN_DISK_PART" --no-user-interaction 2>&1)"
    status=$?

    if [ $status -eq 0 ]; then
        # Successfully mounted now
        MAIN_DISK_PATH=${MAIN_DISK_PATH_S#*at }
        MAIN_DISK_PATH=$(echo "$MAIN_DISK_PATH" | tr -d '.')
        echo "Successfully mounted system partition at $MAIN_DISK_PATH"
        break
    fi

    # Check if already mounted - extract path between backtick and single quote
    # Format: "...is already mounted at \`/run/media/user/xxx'."
    case "$MAIN_DISK_PATH_S" in
        *AlreadyMounted*)
            MAIN_DISK_PATH="${MAIN_DISK_PATH_S#*\`}"
            MAIN_DISK_PATH="${MAIN_DISK_PATH%%\'*}"
            if [ -n "$MAIN_DISK_PATH" ] && [ -d "$MAIN_DISK_PATH" ]; then
                echo "System partition already mounted at $MAIN_DISK_PATH (using existing mount)"
                break
            fi
            ;;
    esac

    echo "Mount failed, retrying..."
    sleep 1
done

if [ -z "$MAIN_DISK_PATH" ]; then
    echo "Failed to mount system partition after 5 attempts"
    udisksctl loop-delete -b $LOOP_DISK
    exit 1
fi
cp -r .cutekit/wingos-disk/* $MAIN_DISK_PATH
if [ $? -ne 0 ]; then
    echo "Failed to copy files to system partition"
    udisksctl unmount -b $MAIN_DISK_PART
    udisksctl loop-delete -b $LOOP_DISK
    exit 1
fi
sync

echo "Unmounting system partition $MAIN_DISK_PART"
udisksctl unmount -b $MAIN_DISK_PART

if [ $? -ne 0 ]; then
    echo "Failed to unmount system partition $MAIN_DISK_PART"
    udisksctl loop-delete -b $LOOP_DISK
    exit 1
fi

echo "Successfully unmounted system partition $MAIN_DISK_PART"

sync

echo "Successfully created disk image at $DISK"

udisksctl loop-delete -b $LOOP_DISK
if [ $? -ne 0 ]; then
    echo "Warning: Failed to delete loop device $LOOP_DISK, but disk image was created successfully"
fi

exit 0
