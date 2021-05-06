# for porting app we need sysroot


mkdir -p ./sysroot/
mkdir -p ./sysroot/libs
mkdir -p ./sysroot/libs/kern
mkdir -p ./sysroot/libs/gui
mkdir -p ./sysroot/libs/utils
cp -r ./libs/libc/** ./sysroot/libs/

cp -r ./libs/kern ./sysroot/libs/
cp -r ./libs/gui ./sysroot/libs/
cp -r ./libs/utils ./sysroot/libs/
cp -r ./libs/module ./sysroot/module/