#!/usr/bin/env bash

set -Eeuo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd -- "${SCRIPT_DIR}/../.." && pwd)"
BUILD_ROOT="${SCRIPT_DIR}/build"
SDK_VERSION="26.5"
OPENSSL_VERSION="3.5.9"
THREADS=8
BUILD_CFG="release"
CLEAN_ONLY=0

usage() {
  cat <<EOF
Usage: $(basename "$0") [-o openssl_version] [-s sdk_version] [-t threads] [-b build_type] [-c] [-h]

Build OpenSSL and the SUPLA client for iOS, then copy both XCFrameworks to SUPLA_IOS.

Options:
  -o VERSION  OpenSSL version (default: 3.5.9)
  -s VERSION  iOS SDK version (default: 26.5)
  -t NUMBER   Number of parallel build jobs (default: 8)
  -b TYPE     Client build configuration: release or debug (default: release)
  -c          Remove the complete local build directory and exit
  -h          Show this help
EOF
}

error() { printf '[ERROR] %s\n' "$*" >&2; }
info() { printf '[INFO] %s\n' "$*"; }
fail() { error "$*"; exit 1; }

show_configuration() {
  printf '[INFO] Build configuration:\n'
  printf '  [-o] OpenSSL version: %s (default: 3.5.9)\n' "$OPENSSL_VERSION"
  printf '  [-s] iOS SDK version: %s (default: 26.5)\n' "$SDK_VERSION"
  printf '  [-t] Parallel jobs: %s (default: 8)\n' "$THREADS"
  printf '  [-b] Build type: %s (default: release)\n' "$BUILD_CFG"
  printf '  [-c] Clean build directory only: %s (default: no)\n' "$CLEAN_ONLY"
  printf '  [$SUPLA_IOS] Destination directory: %s (environment variable)\n' "${SUPLA_IOS:-<not set>}"
}

on_error() {
  local status=$?
  error "Command failed at line ${BASH_LINENO[0]} (exit code ${status})."
  exit "$status"
}
trap on_error ERR

while getopts ':o:s:t:b:ch' option; do
  case "$option" in
    o) OPENSSL_VERSION="$OPTARG" ;;
    s) SDK_VERSION="$OPTARG" ;;
    t) THREADS="$OPTARG" ;;
    b)
      case "$OPTARG" in
        release|Release|RELEASE) BUILD_CFG="release" ;;
        debug|Debug|DEBUG) BUILD_CFG="debug" ;;
        *) fail "Build type must be 'release' or 'debug'." ;;
      esac
      ;;
    c) CLEAN_ONLY=1 ;;
    h) usage; exit 0 ;;
    :) fail "Option -${OPTARG} requires a value." ;;
    \?) fail "Unknown option: -${OPTARG}. Use -h for help." ;;
  esac
done
shift $((OPTIND - 1))
[[ $# -eq 0 ]] || fail "Unexpected argument: $1. Use -h for help."

show_configuration

if [[ "$CLEAN_ONLY" -eq 1 ]]; then
  rm -rf "$BUILD_ROOT"
  info "Removed build directory: ${BUILD_ROOT}"
  exit 0
fi

[[ "$THREADS" =~ ^[1-9][0-9]*$ ]] || fail "Thread count must be a positive integer."
[[ "$OPENSSL_VERSION" =~ ^[0-9]+\.[0-9]+\.[0-9]+([a-z]+[0-9]*)?$ ]] || fail "Invalid OpenSSL version: ${OPENSSL_VERSION}"
[[ "$SDK_VERSION" =~ ^[0-9]+(\.[0-9]+){1,2}$ ]] || fail "Invalid SDK version: ${SDK_VERSION}"
[[ "$BUILD_CFG" == "release" || "$BUILD_CFG" == "debug" ]] || fail "Build type must be 'release' or 'debug'."

[[ -n "${SUPLA_IOS:-}" ]] || fail "SUPLA_IOS is not set. Set it to the destination iOS project directory."
[[ -d "$SUPLA_IOS" ]] || fail "SUPLA_IOS does not point to an existing directory: ${SUPLA_IOS}"

for tool in xcode-select xcodebuild xcrun make curl tar lipo libtool; do
  command -v "$tool" >/dev/null 2>&1 || fail "Required tool not found: ${tool}"
done
[[ -x /usr/libexec/PlistBuddy ]] || fail "Required tool not found: /usr/libexec/PlistBuddy"
command -v ditto >/dev/null 2>&1 || fail "Required tool not found: ditto"

DEVELOPER="$(xcode-select -p)"
[[ -d "$DEVELOPER" ]] || fail "Active Xcode developer directory does not exist: ${DEVELOPER}"
IOS_SDK="${DEVELOPER}/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS${SDK_VERSION}.sdk"
SIM_SDK="${DEVELOPER}/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator${SDK_VERSION}.sdk"
[[ -d "$IOS_SDK" ]] || fail "iPhoneOS SDK ${SDK_VERSION} was not found at ${IOS_SDK}"
[[ -d "$SIM_SDK" ]] || fail "iPhoneSimulator SDK ${SDK_VERSION} was not found at ${SIM_SDK}"

if [[ "$BUILD_CFG" == "release" ]]; then
  CLIENT_CONFIG="Release"
else
  CLIENT_CONFIG="Debug"
fi
CLIENT_BUILD_DIR="${REPO_ROOT}/supla-client/${CLIENT_CONFIG}"
[[ -d "$CLIENT_BUILD_DIR" ]] || fail "Client build directory does not exist: ${CLIENT_BUILD_DIR}"

OPENSSL_ARCHIVE="${BUILD_ROOT}/openssl/downloads/openssl-${OPENSSL_VERSION}.tar.gz"
OPENSSL_VARIANT="${BUILD_ROOT}/openssl/versions/openssl-${OPENSSL_VERSION}/sdk-${SDK_VERSION}"
OPENSSL_SOURCE="${OPENSSL_VARIANT}/source/openssl-${OPENSSL_VERSION}"
OPENSSL_INSTALL="${OPENSSL_VARIANT}/install"
OPENSSL_LOGS="${OPENSSL_VARIANT}/logs"
OPENSSL_FRAMEWORK="${OPENSSL_VARIANT}/LibSsl.xcframework"
CLIENT_VARIANT="${BUILD_ROOT}/supla-client/${BUILD_CFG}/sdk-${SDK_VERSION}"
CLIENT_LOGS="${CLIENT_VARIANT}/logs"
CLIENT_FRAMEWORK="${CLIENT_VARIANT}/LibSuplaClient.xcframework"

mkdir -p "${BUILD_ROOT}/openssl/downloads" "${OPENSSL_LOGS}" "${CLIENT_LOGS}"

is_valid_xcframework() {
  local path="$1" device_library="$2" simulator_library="$3"
  [[ -d "$path" && -f "$path/Info.plist" ]] || return 1
  /usr/libexec/PlistBuddy -c 'Print :AvailableLibraries' "$path/Info.plist" >/dev/null 2>&1 || return 1
  [[ -d "$path/ios-arm64" && -f "$path/ios-arm64/${device_library}" ]] || return 1
  [[ -d "$path/ios-arm64_x86_64-simulator" && -f "$path/ios-arm64_x86_64-simulator/${simulator_library}" ]]
}

build_openssl_arch() {
  local arch="$1" platform="$2" sdk_path="$3"
  local target="" minimum_flag="" install_path="${OPENSSL_INSTALL}/${platform}-${arch}"
  local log="${OPENSSL_LOGS}/${platform}-${arch}.log"
  if [[ "$platform" == "iPhoneSimulator" ]]; then
    target="iossimulator-xcrun"
    minimum_flag="-mios-simulator-version-min=8.2"
  else
    target="iphoneos-cross"
    minimum_flag="-miphoneos-version-min=8.2"
  fi
  mkdir -p "$install_path"
  info "Building OpenSSL ${OPENSSL_VERSION} for ${platform} ${arch} (SDK ${SDK_VERSION})"
  (
    cd "$OPENSSL_SOURCE"
    make clean >"$log" 2>&1 || true
    export CC="${DEVELOPER}/usr/bin/gcc -arch ${arch}"
    ./Configure "$target" "$minimum_flag" \
      "--prefix=${install_path}" "-arch ${arch}" "-isysroot ${sdk_path}" >>"$log" 2>&1
    grep -q 'OPENSSL_THREADS' include/openssl/configuration.h || {
      error "OpenSSL thread support is missing; see ${log}"; return 1;
    }
    make -j"$THREADS" >>"$log" 2>&1
    make install_sw install_ssldirs >>"$log" 2>&1
    make clean >>"$log" 2>&1
  ) || fail "OpenSSL build failed for ${platform} ${arch}. See ${log}"
  [[ -f "${install_path}/lib/libcrypto.a" && -f "${install_path}/lib/libssl.a" ]] || fail "OpenSSL libraries are missing after building ${platform} ${arch}. See ${log}"
}

if is_valid_xcframework "$OPENSSL_FRAMEWORK" libssl-ios.a libssl-simulator.a; then
  info "Using cached OpenSSL XCFramework for version ${OPENSSL_VERSION}, SDK ${SDK_VERSION}"
else
  if [[ ! -f "$OPENSSL_ARCHIVE" ]]; then
    info "OpenSSL archive is missing; removing incomplete build data and downloading sources"
    rm -rf "${OPENSSL_VARIANT}"
    mkdir -p "$(dirname "$OPENSSL_ARCHIVE")"
    curl --fail --location --retry 3 \
      "https://github.com/openssl/openssl/releases/download/openssl-${OPENSSL_VERSION}/openssl-${OPENSSL_VERSION}.tar.gz" \
      --output "${OPENSSL_ARCHIVE}" || fail "Failed to download OpenSSL ${OPENSSL_VERSION}."
  else
    info "OpenSSL sources are available; rebuilding incomplete or missing SDK-specific artifacts"
    rm -rf "${OPENSSL_VARIANT}"
  fi
  mkdir -p "${OPENSSL_SOURCE}" "${OPENSSL_INSTALL}" "${OPENSSL_LOGS}"
  tar -xzf "$OPENSSL_ARCHIVE" -C "$(dirname "$OPENSSL_SOURCE")" || fail "Failed to extract ${OPENSSL_ARCHIVE}"
  [[ -d "$OPENSSL_SOURCE" ]] || fail "OpenSSL source directory was not created: ${OPENSSL_SOURCE}"
  build_openssl_arch arm64 iPhoneOS "$IOS_SDK"
  build_openssl_arch arm64 iPhoneSimulator "$SIM_SDK"
  build_openssl_arch x86_64 iPhoneSimulator "$SIM_SDK"

  info "Combining OpenSSL static libraries"
  lipo -create \
    "${OPENSSL_INSTALL}/iPhoneSimulator-arm64/lib/libcrypto.a" \
    "${OPENSSL_INSTALL}/iPhoneSimulator-x86_64/lib/libcrypto.a" \
    -output "${OPENSSL_VARIANT}/libcrypto-simulator.a"
  lipo -create \
    "${OPENSSL_INSTALL}/iPhoneSimulator-arm64/lib/libssl.a" \
    "${OPENSSL_INSTALL}/iPhoneSimulator-x86_64/lib/libssl.a" \
    -output "${OPENSSL_VARIANT}/libssl-raw-simulator.a"
  libtool -static -no_warning_for_no_symbols -o "${OPENSSL_VARIANT}/libssl-ios.a" \
    "${OPENSSL_INSTALL}/iPhoneOS-arm64/lib/libcrypto.a" \
    "${OPENSSL_INSTALL}/iPhoneOS-arm64/lib/libssl.a"
  libtool -static -no_warning_for_no_symbols -o "${OPENSSL_VARIANT}/libssl-simulator.a" \
    "${OPENSSL_VARIANT}/libcrypto-simulator.a" "${OPENSSL_VARIANT}/libssl-raw-simulator.a"
  rm -rf "$OPENSSL_FRAMEWORK"
  xcodebuild -create-xcframework \
    -library "${OPENSSL_VARIANT}/libssl-ios.a" \
    -headers "${OPENSSL_INSTALL}/iPhoneOS-arm64/include" \
    -library "${OPENSSL_VARIANT}/libssl-simulator.a" \
    -headers "${OPENSSL_INSTALL}/iPhoneSimulator-arm64/include" \
    -output "$OPENSSL_FRAMEWORK" || fail "Failed to create ${OPENSSL_FRAMEWORK}"
  is_valid_xcframework "$OPENSSL_FRAMEWORK" libssl-ios.a libssl-simulator.a || fail "OpenSSL XCFramework is incomplete: ${OPENSSL_FRAMEWORK}"
fi

build_client_arch() {
  local arch="$1" platform="$2" sdk_path="$3"
  local target_dir="${platform}_${arch}"
  local log="${CLIENT_LOGS}/${target_dir}.log"
  local platform_flags=""
  local cpu=""
  case "$arch" in
    arm64) cpu="ARM64" ;;
    x86_64) cpu="X86_64" ;;
    *) fail "Unsupported architecture: ${arch}" ;;
  esac
  if [[ "$platform" == "iPhoneSimulator" ]]; then
    platform_flags="-DTARGET_OS_SIMULATOR=1 -DTARGET_CPU_${cpu} -mios-simulator-version-min=8.2"
  else
    platform_flags="-DTARGET_OS_EMBEDDED=1 -DTARGET_CPU_${cpu} -miphoneos-version-min=8.2"
  fi
  info "Building SUPLA client for ${platform} ${arch} (${BUILD_CFG})"
  (
    cd "$CLIENT_BUILD_DIR"
    mkdir -p "$target_dir"
    export CROSS_TOP="${DEVELOPER}/Platforms/${platform}.platform/Developer"
    export CROSS_SDK="${platform}${SDK_VERSION}.sdk"
    export PARAMS="${platform_flags} -DNOMYSQL -DSRPC_WITHOUT_IN_QUEUE -DSRPC_WITHOUT_OUT_QUEUE -DSPROTO_WITHOUT_OUT_BUFFER -DUSE_DEPRECATED_EMEV_V1 -DUSE_DEPRECATED_EMEV_V2 -DTARGET_OS_IOS=1 -fembed-bitcode -I${OPENSSL_INSTALL}/${platform}-${arch}/include -arch ${arch} -isysroot ${sdk_path}"
    make clean >"$log" 2>&1
    make -j"$THREADS" all >>"$log" 2>&1
    [[ -f libsupla-client.a ]] || { error "Client archive was not produced; see ${log}"; return 1; }
    cp libsupla-client.a "${CLIENT_VARIANT}/${target_dir}/libsupla-client.a"
  ) || fail "SUPLA client build failed for ${platform} ${arch}. See ${log}"
}

rm -rf "$CLIENT_VARIANT"
mkdir -p "${CLIENT_VARIANT}/iPhoneOS_arm64" \
  "${CLIENT_VARIANT}/iPhoneSimulator_arm64" \
  "${CLIENT_VARIANT}/iPhoneSimulator_x86_64" "$CLIENT_LOGS"
build_client_arch arm64 iPhoneOS "$IOS_SDK"
build_client_arch arm64 iPhoneSimulator "$SIM_SDK"
build_client_arch x86_64 iPhoneSimulator "$SIM_SDK"

info "Combining SUPLA client static libraries"
cp "${CLIENT_VARIANT}/iPhoneOS_arm64/libsupla-client.a" "${CLIENT_VARIANT}/libsupla-client-iphoneos.a"
lipo -create \
  "${CLIENT_VARIANT}/iPhoneSimulator_arm64/libsupla-client.a" \
  "${CLIENT_VARIANT}/iPhoneSimulator_x86_64/libsupla-client.a" \
  -output "${CLIENT_VARIANT}/libsupla-client-iphonesimulator.a"
mkdir -p "${CLIENT_VARIANT}/include"
cp "${REPO_ROOT}/supla-client/src/supla-client.h" "${CLIENT_VARIANT}/include/"
cp "${REPO_ROOT}/supla-common/proto.h" "${CLIENT_VARIANT}/include/"
rm -rf "$CLIENT_FRAMEWORK"
xcodebuild -create-xcframework \
  -library "${CLIENT_VARIANT}/libsupla-client-iphoneos.a" \
  -headers "${CLIENT_VARIANT}/include" \
  -library "${CLIENT_VARIANT}/libsupla-client-iphonesimulator.a" \
  -headers "${CLIENT_VARIANT}/include" \
  -output "$CLIENT_FRAMEWORK" || fail "Failed to create ${CLIENT_FRAMEWORK}"
is_valid_xcframework "$CLIENT_FRAMEWORK" libsupla-client-iphoneos.a libsupla-client-iphonesimulator.a || fail "SUPLA client XCFramework is incomplete: ${CLIENT_FRAMEWORK}"

info "Copying XCFrameworks to ${SUPLA_IOS}"
rm -rf "${SUPLA_IOS}/LibSsl.xcframework" "${SUPLA_IOS}/LibSuplaClient.xcframework"
ditto "$OPENSSL_FRAMEWORK" "${SUPLA_IOS}/LibSsl.xcframework" || fail "Failed to copy LibSsl.xcframework to ${SUPLA_IOS}"
ditto "$CLIENT_FRAMEWORK" "${SUPLA_IOS}/LibSuplaClient.xcframework" || fail "Failed to copy LibSuplaClient.xcframework to ${SUPLA_IOS}"
info "Build completed successfully. Frameworks are available in ${SUPLA_IOS}"
