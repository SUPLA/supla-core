#!/bin/bash


# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.


SDK_VERSION="8.2"
BUILD_CFG="Release"
IOS_OPENSSL_LIB_INC="/Users/przemek/CProjects/openssl/OpenSSL-for-iPhone/include"

if [ ! -z $1 ]; then

   BUILD_CFG=$1

   if [ ! -z $2  ]; then
     SDK_VERSION=$2  
   fi
fi

BUILD_DIR="../"$BUILD_CFG
DEVELOPER=`xcode-select -print-path`

if [ ! -d $BUILD_DIR ]; then
    echo "Build dir does not exists! " $BUILD_DIR;
    exit;
fi

build()
{
	ARCH=$1
        PARAMS=""

        if [[ "${ARCH}" == "armv7" || "${ARCH}" == "armv7s" ]]; then
           TARGET_CPU="ARM"
        else
           TARGET_CPU=`echo -n ${ARCH} | awk '{ print toupper($0) }'`
        fi
  
	if [[ "${ARCH}" == "i386" || "${ARCH}" == "x86_64" ]]; then
		PLATFORM="iPhoneSimulator"
                PARAMS="-DTARGET_OS_SIMULATOR=1 -DTARGET_CPU_${TARGET_CPU} "
	else
		PLATFORM="iPhoneOS"
                PARAMS="-DTARGET_OS_EMBEDDED=1 -DTARGET_CPU_${TARGET_CPU} "
	fi

        cd $BUILD_DIR

        export CROSS_TOP="${DEVELOPER}/Platforms/${PLATFORM}.platform/Developer"
        export CROSS_SDK="${PLATFORM}${SDK_VERSION}.sdk"

        export PARAMS=${PARAMS}"-DTARGET_OS_IOS=1 -fembed-bitcode -I${IOS_OPENSSL_LIB_INC} -arch ${ARCH} -isysroot ${CROSS_TOP}/SDKs/${CROSS_SDK} -miphoneos-version-min=${SDK_VERSION}"
        
        make clean
        make

        mkdir -p ${ARCH}
        mv libsupla-client.a ${ARCH}
       
};

CWD=`pwd`

build "armv7"
build "armv7s"
build "arm64"
build "x86_64"
build "i386"

echo $CWD
cd $CWD

lipo \
	"${BUILD_DIR}/armv7s/libsupla-client.a" \
	"${BUILD_DIR}/armv7/libsupla-client.a" \
	"${BUILD_DIR}/arm64/libsupla-client.a" \
	"${BUILD_DIR}/x86_64/libsupla-client.a" \
	"${BUILD_DIR}/i386/libsupla-client.a" \
	-create -output "${BUILD_DIR}/libsupla-client.a" \
&& 
echo "${BUILD_DIR}/libsupla-client.a"
