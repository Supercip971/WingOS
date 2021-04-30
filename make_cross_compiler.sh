
#!/bin/bash
set -e

# using : https://wiki.osdev.org/Cross-Compiler_Successful_Builds
echo "creating wingOS_cross Compiler"

OK_FILE="$PWD/cross_compiler/builded"
bash ./make_sysroot.sh
if [ -e $OK_FILE ]; then
    echo "cross compiler already builded ! if you want to rebuild the cross compiler please remove the './cross_compiler/builded' file"
    exit 0
fi

SYSROOT="$PWD/sysroot/"
mkdir -p $SYSROOT
download_and_extract() {
        echo "Downloading $2"
        wget -c "$1"
        echo "Extracting $2"
        tar xf "$2"
}
patch_path="$PWD/patch/"


bin_utils_version="2.33.1"
binutils_file="binutils-$bin_utils_version.tar.xz"

gcc_version="10.1.0"
gcc_file="gcc-$gcc_version.tar.xz"

export PREFIX="$PWD/cross_compiler"
export TARGET="x86_64-pc-wingos"
export PATH="$PREFIX/bin:$PATH"

mkdir -p ./cross_compiler/build/gcc ./cross_compiler/build/binutils
mkdir -p ./cross_compiler/src

cd ./cross_compiler/src

echo "downloading bin_utils"
download_and_extract "https://ftp.gnu.org/gnu/binutils/$binutils_file" "$binutils_file"
export binutils_src="$PWD/binutils-$bin_utils_version"
cd $binutils_src
patch -p1 < "$patch_path/binutils.patch"
cd ..




echo "downloading gcc"
download_and_extract "ftp://ftp.gnu.org/gnu/gcc/gcc-$gcc_version/$gcc_file" "$gcc_file"

export gcc_src="$PWD/gcc-$gcc_version"
cd $gcc_src
patch -p1 < "$patch_path/gcc.patch"
cd ..



cd "gcc-$gcc_version"
./contrib/download_prerequisites
cd ..


cd ..
cd build

echo "building bin_utils"
cd binutils
"$binutils_src/configure" --target="$TARGET" 	\
        --prefix="$PREFIX" 	\
        --with-sysroot=$SYSROOT		\
        --disable-nls 		\
        --disable-werror

make -j$(nproc)
make install -j$(nproc)
cd ..

echo "building gcc"
cd gcc
"$gcc_src/configure" --target="$TARGET" \
        --prefix="$PREFIX" 		\
        --disable-nls			\
        --enable-languages=c,c++	\
        --with-newlib \
        --with-sysroot=$SYSROOT

make all-gcc -j$(nproc)
make all-target-libgcc -j$(nproc)
make install-gcc -j$(nproc)
make install-target-libgcc -j$(nproc)

cd ..

touch $OK_FILE