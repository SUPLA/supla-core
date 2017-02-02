#!/bin/bash

# ============================================================================
# Name        : build_ios.sh
# Author      : Przemyslaw Zygmunt p.zygmunt@acsoftware.pl [AC SOFTWARE]
# Version     : 1.0
# Copyright   : GPLv2
# ============================================================================


SDK_VERSION="8.4"
BUILD_CFG="Release"
IOS_OPENSSL_LIB_INC="../../../openssl/include"

if [ ! -z $1 ]; then

   BUILD_CFG=$1

   if [ ! -z $2  ]; then
     SDK_VERSION=$2  
   fi
fi

BUILD_DIR="./"$BUILD_CFG
DEVELOPER=`xcode-select -print-path`

if [ ! -d $BUILD_DIR ]; then
    echo "Build dir does not exists! " $BUILD_DIR;
    exit;
fi

build()
{
	ARCH=$1
  
	if [[ "${ARCH}" == "i386" || "${ARCH}" == "x86_64" ]]; then
		PLATFORM="iPhoneSimulator"
	else
		PLATFORM="iPhoneOS"
	fi

        cd $BUILD_DIR

        export CROSS_TOP="${DEVELOPER}/Platforms/${PLATFORM}.platform/Developer"
        export CROSS_SDK="${PLATFORM}${SDK_VERSION}.sdk"

        export PARAMS="-fembed-bitcode -I${IOS_OPENSSL_LIB_INC} -arch ${ARCH} -isysroot ${CROSS_TOP}/SDKs/${CROSS_SDK} -miphoneos-version-min=${SDK_VERSION}"

        make clean
        make

        mkdir -p ${ARCH}
        mv libsupla-client.a ${ARCH}
       
};

CWD=`pwd`

build "armv7"
build "arm64"
build "x86_64"
build "i386"

echo $CWD
cd $CWD

lipo \
	"${BUILD_DIR}/armv7/libsupla-client.a" \
	"${BUILD_DIR}/arm64/libsupla-client.a" \
	"${BUILD_DIR}/x86_64/libsupla-client.a" \
	"${BUILD_DIR}/i386/libsupla-client.a" \
	-create -output "${BUILD_DIR}/libsupla-client.a"

