#!/bin/bash

# This script builds the iOS openSSL libraries
# SDK8.2 (XCode 6.2)

set -e

# Defaults
SDK="16.4"
OPENSSL="3.0.8"
THREADS=4

temp_clean() {
  rm -rf libcrypto-iphoneos.a libcrypto-iphonesimulator.a \
    libssl-iphoneos.a libssl-iphonesimulator.a \
    libssl-os.a libssl-sim.a
}

clean() {
  echo "[INFO] Cleaning up" 
  rm -rf LibSsl.xcframework
  temp_clean
    
  rm -rf "/tmp/${OPENSSL_VERSION}-*"
  rm -rf "/tmp/${OPENSSL_VERSION}-*.log"
}

while getopts 's:o:t:hc' OPTION
do
  case "$OPTION" in
    s) SDK=${OPTARG};;
    o) OPENSSL=${OPTARG};;
    t) THREADS=${OPTARG};;
    c) 
      clean 
      exit 0
      ;;
    h) 
      echo "usage: $(basename \$0) [-s sdk_version] [-o openssl_version] [-t threads_no]"
      exit 0
      ;;
    ?)
      echo "usage: $(basename \$0) [-s sdk_version] [-o openssl_version] [-t threads_no]"
      exit 0
      ;;
  esac
done

echo "[INFO] Openssl version '${OPENSSL}' will be prepared for sdk version '${SDK}'"

SDK_VERSION=${SDK}
OPENSSL_VERSION="openssl-${OPENSSL}"
DEVELOPER=`xcode-select -print-path`

build() {
  ARCH=$1
  PLATFORM=$2
  LOG_FILE="/tmp/${OPENSSL_VERSION}-${PLATFORM}-${ARCH}.log"
  INSTALL_PATH="/tmp/${OPENSSL_VERSION}-${PLATFORM}-${ARCH}"
  
  echo "[INFO] Building '${OPENSSL_VERSION}' for '${PLATFORM}' with sdk '${SDK_VERSION}' architecture '${ARCH}'"
  
  pushd . > /dev/null
  cd "${OPENSSL_VERSION}"
  
  export CC="${DEVELOPER}/usr/bin/gcc -arch ${ARCH}"
  
  if [[ "${PLATFORM}" == "iPhoneSimulator" ]]
  then 
    ./Configure iossimulator-xcrun -mios-simulator-version-min=8.2 "--prefix=${INSTALL_PATH}" "-arch ${ARCH}" "-isysroot ${DEVELOPER}/Platforms/${PLATFORM}.platform/Developer/SDKs/${PLATFORM}${SDK_VERSION}.sdk" &> ${LOG_FILE}
    RC=$?
  else 
    sed -ie "s!static volatile sig_atomic_t intr_signal;!static volatile intr_signal;!" "crypto/ui/ui_openssl.c"
    ./Configure iphoneos-cross -miphoneos-version-min=8.2 "--prefix=${INSTALL_PATH}" "-arch ${ARCH}" "-isysroot ${DEVELOPER}/Platforms/${PLATFORM}.platform/Developer/SDKs/${PLATFORM}${SDK_VERSION}.sdk" &> ${LOG_FILE}
    RC=$?
  fi
  if [ $RC -ne 0 ]
  then
    echo "[ERROR] Configure failed with rc=${RC}, please check logs ${LOG_FILE}"
    return
  fi

  make -j${THREADS} >> ${LOG_FILE} 2>&1
  RC=$?
  if [ $RC -ne 0 ]
  then
    echo "[ERROR] Make failed with rc=${RC}, please check logs ${LOG_FILE}"
    return
  fi

  make install >> ${LOG_FILE} 2>&1
  RC=$?
  if [ $RC -ne 0 ]
  then
    echo "[ERROR] Install failed with rc=${RC}, please check logs ${LOG_FILE}"
    return
  fi

  make >> ${LOG_FILE} 2>&1
  make install >> ${LOG_FILE} 2>&1
  make clean >> ${LOG_FILE} 2>&1
  popd > /dev/null
}

clean

if [ ! -e ${OPENSSL_VERSION}.tar.gz ]
then
  echo "[INFO] Downloading ${OPENSSL_VERSION}.tar.gz"
  curl -O https://www.openssl.org/source/${OPENSSL_VERSION}.tar.gz
else
  echo "[INFO] Using ${OPENSSL_VERSION}.tar.gz"
fi

if [ ! -d ./${OPENSSL_VERSION} ]
then
  echo "[INFO] Unpacking ${OPENSSL_VERSION}.tar.gz"
  tar xfz "${OPENSSL_VERSION}.tar.gz"
elif [ -f ./${OPENSSL_VERSION}/Makefile ]; then
  echo "[INFO] Cleaning found version ${OPENSSL_VERSION}"
  cd ${OPENSSL_VERSION}
  make clean > /dev/null 2>&1
  cd ..
fi

build "x86_64" "iPhoneSimulator"
build "arm64" "iPhoneSimulator"
build "armv7" "iPhoneOS"
build "arm64" "iPhoneOS"

echo "[INFO] Merging static libraries"
lipo \
  /tmp/${OPENSSL_VERSION}-iPhoneOS-arm64/lib/libcrypto.a \
  /tmp/${OPENSSL_VERSION}-iPhoneOS-armv7/lib/libcrypto.a \
  -create -output libcrypto-iphoneos.a
  
lipo \
  /tmp/${OPENSSL_VERSION}-iPhoneSimulator-arm64/lib/libcrypto.a \
  /tmp/${OPENSSL_VERSION}-iPhoneSimulator-x86_64/lib/libcrypto.a \
  -create -output libcrypto-iphonesimulator.a

lipo \
  /tmp/${OPENSSL_VERSION}-iPhoneOS-arm64/lib/libssl.a \
  /tmp/${OPENSSL_VERSION}-iPhoneOS-armv7/lib/libssl.a \
  -create -output libssl-iphoneos.a
  
lipo \
  /tmp/${OPENSSL_VERSION}-iPhoneSimulator-arm64/lib/libssl.a \
  /tmp/${OPENSSL_VERSION}-iPhoneSimulator-x86_64/lib/libssl.a \
  -create -output libssl-iphonesimulator.a

libtool -static -no_warning_for_no_symbols -o libssl-os.a libcrypto-iphoneos.a libssl-iphoneos.a
libtool -static -no_warning_for_no_symbols -o libssl-sim.a libcrypto-iphonesimulator.a libssl-iphonesimulator.a

echo "[INFO] Creating XCFramework"

xcodebuild -create-xcframework \
  -library libssl-os.a \
  -headers /tmp/${OPENSSL_VERSION}-iPhoneOS-arm64/include \
  -library libssl-sim.a \
  -headers /tmp/${OPENSSL_VERSION}-iPhoneSimulator-arm64/include \
  -output LibSsl.xcframework

temp_clean
