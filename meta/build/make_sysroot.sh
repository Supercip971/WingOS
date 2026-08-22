mkdir -p meta/build/sysroot/lib

cp -r src/* meta/build/sysroot/lib
cp -r ./src/libc/** meta/build/sysroot/lib

# technically should always be here
if [ -d meta/build/cross/x86_64-pc-wingos/lib ]; then
    cp -r meta/build/cross/x86_64-pc-wingos/lib/* meta/build/sysroot/lib/
fi
