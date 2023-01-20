#!/bin/bash

# This script is inspired from build-image-limine.sh from serenity OS
# You can see their license here: https://github.com/SerenityOS/serenity/blob/master/LICENSE
# (BSD 2-Clause "Simplified" License)

set -e
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )


if [ ! -d "limine" ]; then
    echo "limine not found, the script will now build it"
    git clone --depth 1 --branch v4.x-branch-binary --single-branch https://github.com/limine-bootloader/limine
    cd limine
    make all
    cd ..
fi


# Rexecute 

if [ "$UID" -ne 0 ] ; then 
    echo "Please run current script as root"
    exec sudo -E bash "$0" "$@"

    exit 0
fi


DISK_IMAGE="disk.hdd"
LOOPBACK_DEV=""
LOOPBACK_EFI=""
LOOPBACK_ROOT=""

SHOULD_REFORMAT="no"

if [ ! -f "$DISK_IMAGE" ] ; then 
    printf "Disk not founded, creating it \n"
    printf "Creating file... "
    dd if=/dev/zero of=$DISK_IMAGE bs=1M count=680 status=none
    printf "Ok\n"
    # When using sudo SUDO_UID is set to the UID of the user who invoked sudo
    # This is usefull for when you want to run a script as root but still want to
    # have the created file owned by the user who invoked sudo.
    chown "$SUDO_UID":"$SUDO_GID" $DISK_IMAGE



    echo "Using loopback device: $LOOPBACK_DEV"

    SHOULD_REFORMAT="yes"

fi

printf "Creating loopback device... "

LOOPBACK_DEV=$(losetup --find --partscan --show $DISK_IMAGE)
LOOPBACK_EFI="${LOOPBACK_DEV}p1"
LOOPBACK_ROOT="${LOOPBACK_DEV}p2"

printf "Ok\n"
if [ -z "$LOOPBACK_DEV" ] ; then
    echo "Failed to create loopback device"
    echo "  Rarely, Linux is unable to find a loopback device after an update"
    echo "  Maybe rebooting will fix this issue."
    exit 1
fi

echo "Using loopback device: $LOOPBACK_DEV"

MOUNT_ROOT_PATH="./mnt"
MOUNT_EFI_PATH="./esp"


cleanup() {
    printf "syncing..."
    sync
    printf "done\n"
    printf "cleaning up...\n"
    if [ -d "$MOUNT_ROOT_PATH" ]; then
        printf "Unmounting 'root' partition ($MOUNT_ROOT_PATH)... "
        tree "$MOUNT_ROOT_PATH"
        umount -R $MOUNT_ROOT_PATH
        sync 
        rm -rf $MOUNT_ROOT_PATH
        printf "done \n"
    fi
    sync
    if [ -d $MOUNT_EFI_PATH ]; then
        printf "Unmounting 'efi' partition ($MOUNT_EFI_PATH)... "
        tree "$MOUNT_EFI_PATH"
        umount -R $MOUNT_EFI_PATH
        sync
        rm -rf $MOUNT_EFI_PATH
        printf "done \n"
    fi

    if [ -e "${LOOPBACK_DEV}" ]; then
        printf "Cleaning up loopback device... "
        losetup -d "${LOOPBACK_DEV}"
        printf "done \n"
    fi
}


trap cleanup EXIT

if [ "$SHOULD_REFORMAT" == "yes" ] ; then 
printf "Creating partitions..."

EFI_START="1MiB"
EFI_END="90MiB"

ROOT_START="$EFI_END"
ROOT_END="100%"

echo "EFI partition: $EFI_START - $EFI_END"
echo "Root partition: $ROOT_START - $ROOT_END"

parted -s $LOOPBACK_DEV mklabel gpt \
    mkpart EFI fat32 $EFI_START $EFI_END \
    set 1 esp on \
    mkpart ROOT ext2 $ROOT_START $ROOT_END

printf "Ok\n"
echo "Formatting partitions..."

printf "Formating uefi partition..."
mkfs.vfat -F 32 "${LOOPBACK_EFI}"
printf "Ok\n"


echo "Formating root partition..."
mkfs.ext2 -L "ROOT" "${LOOPBACK_ROOT}"

printf "Ok\n"

    printf "Mounting filesystems... "
    mkdir -p $MOUNT_EFI_PATH
    mount "${LOOPBACK_EFI}" $MOUNT_EFI_PATH 
    mkdir -p mnt
    mount "${LOOPBACK_ROOT}" $MOUNT_ROOT_PATH
    printf "Ok\n"


else 
    printf "Mounting filesystems... "
    mkdir -p $MOUNT_EFI_PATH
    mount "${LOOPBACK_EFI}" $MOUNT_EFI_PATH 
    mkdir -p mnt
    mount "${LOOPBACK_ROOT}" $MOUNT_ROOT_PATH
    printf "Ok\n"

    printf "Removing old files..."

    rm -rf "$MOUNT_ROOT_PATH/**"
    rm -rf "$MOUNT_EFI_PATH/**"


    printf "Ok\n"

fi

BUILD_DIR=$BUILD_DIR bash $SCRIPT_DIR/gen-disk.sh

echo "Installing Limine"
mkdir -p "${MOUNT_EFI_PATH}/EFI/BOOT"
cp limine/limine.sys "${MOUNT_EFI_PATH}/"
cp limine/limine.sys "${MOUNT_ROOT_PATH}/"
cp limine/BOOTX64.EFI "${MOUNT_EFI_PATH}/EFI/BOOT"
cp "$SCRIPT_DIR"/../../../disk/limine.cfg "${MOUNT_EFI_PATH}/"


limine/limine-deploy "${LOOPBACK_DEV}"

echo "Script success"
