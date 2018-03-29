#!/bin/sh

rm -rf openssl-1.0.2n
tar zxvf openssl-1.0.2n.tar.gz
cd openssl-1.0.2n

export NDK=/Users/AC/Devel/projekty/Android/ndk-r14b

$NDK/build/tools/make-standalone-toolchain.sh --platform=android-12 --toolchain=x86-4.6 --install-dir=`pwd`/android-toolchain-x86
export TOOLCHAIN_PATH=`pwd`/android-toolchain-x86/bin
export TOOL=i686-linux-android
export NDK_TOOLCHAIN_BASENAME=${TOOLCHAIN_PATH}/${TOOL}
export CC=$NDK_TOOLCHAIN_BASENAME-gcc
export CXX=$NDK_TOOLCHAIN_BASENAME-g++
export LINK=${CXX}
export LD=$NDK_TOOLCHAIN_BASENAME-ld
export AR=$NDK_TOOLCHAIN_BASENAME-ar
export RANLIB=$NDK_TOOLCHAIN_BASENAME-ranlib
export STRIP=$NDK_TOOLCHAIN_BASENAME-strip
export ARCH_FLAGS="-march=i686 -msse3 -mstackrealign -mfpmath=sse"
export ARCH_LINK=
export CPPFLAGS=" ${ARCH_FLAGS} -fpic -ffunction-sections -funwind-tables -fstack-protector -fno-strict-aliasing -finline-limit=64 "
export CXXFLAGS=" ${ARCH_FLAGS} -fpic -ffunction-sections -funwind-tables -fstack-protector -fno-strict-aliasing -finline-limit=64 -frtti -fexceptions "
export CFLAGS=" ${ARCH_FLAGS} -fpic -ffunction-sections -funwind-tables -fstack-protector -fno-strict-aliasing -finline-limit=64 "
export LDFLAGS=" ${ARCH_LINK} "
#export C_INCLUDE_PATH=`pwd`/android-toolchain-x86/sysroot/usr/include
#export CROSS_SYSROOT=`pwd`/android-toolchain-x86/sysroot

./Configure android-x86 no-asm 
PATH=$TOOLCHAIN_PATH:$PATH make

mv lib*.a ../x86/
