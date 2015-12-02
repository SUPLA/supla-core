#!/bin/bash

# ============================================================================
# Name        : build_android.sh
# Author      : Przemyslaw Zygmunt p.zygmunt@acsoftware.pl [AC SOFTWARE]
# Version     : 1.0
# Copyright   : GPLv2
# ============================================================================

NDK_ROOT="/Users/AC/Devel/projekty/Android/ndk-r10c"

export OPENSSL_ANDROID=/Users/AC/Public/openssl

$NDK_ROOT/ndk-build clean
$NDK_ROOT/ndk-build -C ./ NDK_DEBUG=1

cp  libs/armeabi/libsuplaclient.so ~/AndroidStudioProjects/SUPLA/app/src/main/libs/armeabi
cp  libs/armeabi-v7a/libsuplaclient.so ~/AndroidStudioProjects/SUPLA/app/src/main/libs/armeabi-v7a
cp  libs/x86/libsuplaclient.so ~/AndroidStudioProjects/SUPLA/app/src/main/libs/x86
