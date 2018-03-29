#!/bin/sh

rm -rf openssl-1.0.2n
tar zxvf openssl-1.0.2n.tar.gz
cd openssl-1.0.2n

export NDK=/Users/AC/Devel/projekty/Android/ndk-r14b
$NDK/build/tools/make-standalone-toolchain.sh --platform=android-12 --toolchain=arm-linux-androideabi-4.8 --install-dir=`pwd`/android-toolchain-arm
export TOOLCHAIN_PATH=`pwd`/android-toolchain-arm/bin
export TOOL=arm-linux-androideabi
export NDK_TOOLCHAIN_BASENAME=${TOOLCHAIN_PATH}/${TOOL}
export CC=$NDK_TOOLCHAIN_BASENAME-gcc
export CXX=$NDK_TOOLCHAIN_BASENAME-g++
export LINK=${CXX}
export LD=$NDK_TOOLCHAIN_BASENAME-ld
export AR=$NDK_TOOLCHAIN_BASENAME-ar
export RANLIB=$NDK_TOOLCHAIN_BASENAME-ranlib
export STRIP=$NDK_TOOLCHAIN_BASENAME-strip
export ARCH_FLAGS="-march=armv7-a -mfloat-abi=softfp -mfpu=vfpv3-d16"
export ARCH_LINK="-march=armv7-a -Wl,--fix-cortex-a8"
export CPPFLAGS=" ${ARCH_FLAGS} -fpic -ffunction-sections -funwind-tables -fstack-protector -fno-strict-aliasing -finline-limit=64 "
export CXXFLAGS=" ${ARCH_FLAGS} -fpic -ffunction-sections -funwind-tables -fstack-protector -fno-strict-aliasing -finline-limit=64 -frtti -fexceptions "
export CFLAGS=" ${ARCH_FLAGS} -fpic -ffunction-sections -funwind-tables -fstack-protector -fno-strict-aliasing -finline-limit=64 "
export LDFLAGS=" ${ARCH_LINK} "
#export C_INCLUDE_PATH=`pwd`/android-toolchain-arm/sysroot/usr/include
#export CROSS_SYSROOT=`pwd`/android-toolchain-arm/sysroot

./Configure android-armv7
PATH=$TOOLCHAIN_PATH:$PATH make

mv lib*.a ../armeabi-v7a/
