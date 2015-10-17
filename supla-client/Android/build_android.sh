#!/bin/bash

# ============================================================================
# Name        : build_android.sh
# Author      : Przemyslaw Zygmunt p.zygmunt@acsoftware.pl [AC SOFTWARE]
# Version     : 1.0
# Copyright   : GPLv2
# ============================================================================

NDK_ROOT="/Users/AC/Devel/projekty/Android/ndk-r10c"
BUILD_CFG="../Release"
IOS_OPENSSL_LIB_INC="../../../openssl/include"

BUILD_DIR="./"$BUILD_CFG
DEVELOPER=`xcode-select -print-path`

if [ ! -d $BUILD_DIR ]; then
    echo "Build dir does not exists! " $BUILD_DIR;
    exit;
fi

PATH="$NDK_ROOT/toolchains/arm-linux-androideabi-4.9/prebuilt/darwin-x86_64/bin/:$PATH"
export SYS_ROOT="$NDK_ROOT/platforms/android-12/arch-arm/"
export GCC="arm-linux-androideabi-gcc -march=armv7-a -I${IOS_OPENSSL_LIB_INC} --sysroot=$SYS_ROOT"
export CC=$GCC
export LD="arm-linux-androideabi-ld"
export AR="arm-linux-androideabi-ar"
export RANLIB="arm-linux-androideabi-ranlib"
export STRIP="arm-linux-androideabi-strip"
export OPENSSL_ANDROID=/Users/AC/Public/openssl

P=`pwd`

cd $BUILD_DIR
make clean
make

cd $P

$NDK_ROOT/ndk-build clean
$NDK_ROOT/ndk-build -C ./ NDK_DEBUG=1

cp  libs/armeabi/libsuplaclient.so ~/AndroidStudioProjects/SUPLA/app/src/main/libs/armeabi
cp  libs/armeabi-v7a/libsuplaclient.so ~/AndroidStudioProjects/SUPLA/app/src/main/libs/armeabi-v7a
cp  libs/x86/libsuplaclient.so ~/AndroidStudioProjects/SUPLA/app/src/main/libs/x86
