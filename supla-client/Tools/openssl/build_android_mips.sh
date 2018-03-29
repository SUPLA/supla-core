#!/bin/sh

rm -r openssl-1.0.1u
tar zxvf openssl-1.0.1u.tar.gz
cd openssl-1.0.1u


export NDK=/Users/AC/Devel/projekty/Android/ndk-r10c

$NDK/build/tools/make-standalone-toolchain.sh --platform=android-12 --toolchain=mipsel-linux-android-4.8 --install-dir=`pwd`/android-toolchain-mips
export TOOLCHAIN_PATH=`pwd`/android-toolchain-mips/bin
export TOOL=mipsel-linux-android
export NDK_TOOLCHAIN_BASENAME=${TOOLCHAIN_PATH}/${TOOL}
export CC=$NDK_TOOLCHAIN_BASENAME-gcc
export CXX=$NDK_TOOLCHAIN_BASENAME-g++
export LINK=${CXX}
export LD=$NDK_TOOLCHAIN_BASENAME-ld
export AR=$NDK_TOOLCHAIN_BASENAME-ar
export RANLIB=$NDK_TOOLCHAIN_BASENAME-ranlib
export STRIP=$NDK_TOOLCHAIN_BASENAME-strip
export ARCH_FLAGS="-mthumb"
export ARCH_LINK=
export CPPFLAGS=" ${ARCH_FLAGS} -fpic -ffunction-sections -funwind-tables -fstack-protector -fno-strict-aliasing -finline-limit=64 "
export CXXFLAGS=" ${ARCH_FLAGS} -fpic -ffunction-sections -funwind-tables -fstack-protector -fno-strict-aliasing -finline-limit=64 -frtti -fexceptions "
export CFLAGS=" ${ARCH_FLAGS} -fpic -ffunction-sections -funwind-tables -fstack-protector -fno-strict-aliasing -finline-limit=64 "
export LDFLAGS=" ${ARCH_LINK} "
./Configure android
PATH=$TOOLCHAIN_PATH:$PATH make

mv lib*.a ../mips/
