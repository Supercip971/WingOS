#!/bin/sh
set -e


# We expect to have: mnt/

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )


echo "using build dir: $BUILD_DIR"
TARGET_DIR="mnt"
cp -rT "$SCRIPT_DIR"/../../../disk/ "${TARGET_DIR}/"
#cp "$SCRIPT_DIR"/../../../disk/ "${TARGET_DIR}/"

cp -rT "$BUILD_DIR"/root-path/ "${TARGET_DIR}/"
cp -rT "$BUILD_DIR"/root-path/boot/ "esp"
