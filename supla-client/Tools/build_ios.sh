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


SDK="16.2"
OPENSSL=""
BUILD_CFG="Release"
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

while getopts 's:o:t:b:h' OPTION
do
  case "$OPTION" in
    s) SDK=${OPTARG};;
    o) OPENSSL=${OPTARG};;
    t) THREADS=${OPTARG};;
    b) BUILD_CFG=${OPTARG};;
    h) 
      echo "usage: $(basename \$0) [-s sdk_version] [-o openssl_headers] [-t threads_no] [-b build_type]"
      exit 0
      ;;
    ?)
      echo "usage: $(basename \$0) [-s sdk_version] [-o openssl_headers] [-t threads_no] [-b build_type"
      exit 0
      ;;
  esac
done

if [ -z "$OPENSSL" ]
then
  echo "[ERROR] Mandatory path to openssl missing"
  exit 1
fi

SDK_VERSION=${SDK}
BUILD_DIR="${SCRIPT_DIR}/../${BUILD_CFG}"
DEVELOPER=`xcode-select -print-path`
IOS_OPENSSL_LIB_INC="$OPENSSL"

if [ ! -d $BUILD_DIR ]; then
    echo "[ERROR] Build dir does not exists! " $BUILD_DIR
    exit 1
fi

echo "[INFO] Supla client will be prepared for sdk version '${SDK}' inside ${BUILD_CFG} config"

build()
{
  ARCH=$1
  PLATFORM=$2
  PARAMS=""
  TARGET_DIR="${PLATFORM}_${ARCH}"
  LOG_FILE="${TARGET_DIR}/build.log"
  
  echo "[INFO] Building '${PLATFORM}' for architecture '${ARCH}'"

  if [[ "${ARCH}" == "armv7" || "${ARCH}" == "armv7s" ]]; then
     TARGET_CPU="ARM"
  else
     TARGET_CPU=`echo -n ${ARCH} | awk '{ print toupper($0) }'`
  fi
  
  if [[ "${PLATFORM}" == "iPhoneSimulator" ]]; then
    PARAMS="-DTARGET_OS_SIMULATOR=1 -DTARGET_CPU_${TARGET_CPU} -mios-simulator-version-min=8.2  "
  else
    PARAMS="-DTARGET_OS_EMBEDDED=1 -DTARGET_CPU_${TARGET_CPU} -miphoneos-version-min=8.2 "
  fi
  
  cd $BUILD_DIR
  mkdir -p "${PLATFORM}_${ARCH}"

  export CROSS_TOP="${DEVELOPER}/Platforms/${PLATFORM}.platform/Developer"
  export CROSS_SDK="${PLATFORM}${SDK_VERSION}.sdk"

  export PARAMS=${PARAMS}"-DNOMYSQL -DSRPC_WITHOUT_IN_QUEUE -DSRPC_WITHOUT_OUT_QUEUE -DSPROTO_WITHOUT_OUT_BUFFER -DUSE_DEPRECATED_EMEV_V1 -DTARGET_OS_IOS=1 -fembed-bitcode -I${IOS_OPENSSL_LIB_INC} -arch ${ARCH} -isysroot ${CROSS_TOP}/SDKs/${CROSS_SDK}"
  
  make clean > ${LOG_FILE} 2>&1
  make all >> ${LOG_FILE} 2>&1
  RC=$?
  if [ $RC -ne 0 ]
  then
    echo "[ERROR] Make failed with rc=${RC}, please check logs ${LOG_FILE}"
    exit 1
  fi

  mv libsupla-client.a "${PLATFORM}_${ARCH}/"
}

build "armv7" "iPhoneOS"
build "armv7s" "iPhoneOS"
build "arm64" "iPhoneOS"
build "arm64" "iPhoneSimulator"
build "x86_64" "iPhoneSimulator"

echo "[INFO] Linking libraries"
lipo \
  "${BUILD_DIR}/iPhoneOS_armv7/libsupla-client.a" \
  "${BUILD_DIR}/iPhoneOS_armv7s/libsupla-client.a" \
  "${BUILD_DIR}/iPhoneOS_arm64/libsupla-client.a" \
  -create -output "${BUILD_DIR}/libsupla-client-iphoneos.a"
  
lipo \
  "${BUILD_DIR}/iPhoneSimulator_arm64/libsupla-client.a" \
  "${BUILD_DIR}/iPhoneSimulator_x86_64/libsupla-client.a" \
  -create -output "${BUILD_DIR}/libsupla-client-iphonesimulator.a"

echo "[INFO] Creating XCFramework"
rm -rf "${BUILD_DIR}/LibSuplaClient.xcframework"
mkdir -p ${BUILD_DIR}/include
cp ${BUILD_DIR}/../src/supla-client.h ${BUILD_DIR}/include/
cp ${BUILD_DIR}/../../supla-common/proto.h ${BUILD_DIR}/include/
xcodebuild -create-xcframework \
  -library "${BUILD_DIR}/libsupla-client-iphoneos.a" \
  -headers "${BUILD_DIR}/include" \
  -library "${BUILD_DIR}/libsupla-client-iphonesimulator.a" \
  -headers "${BUILD_DIR}/include" \
  -output "${BUILD_DIR}/LibSuplaClient.xcframework"
  
RC=$?
if [ $RC -eq 0 ]
then
  echo "[INFO] XCFramework created: ${BUILD_DIR}/LibSuplaClient.xcframework"
fi

rm -rf "${BUILD_DIR}/iPhoneOS_armv7" \
  "${BUILD_DIR}/iPhoneOS_armv7s" \
  "${BUILD_DIR}/iPhoneOS_arm64" \
  "${BUILD_DIR}/iPhoneSimulator_arm64" \
  "${BUILD_DIR}/iPhoneSimulator_x86_64" \
  "${BUILD_DIR}/libsupla-client-iphoneos.a" \
  "${BUILD_DIR}/libsupla-client-iphonesimulator.a" \
  "${BUILD_DIR}/include"
