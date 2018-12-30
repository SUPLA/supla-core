LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := crypto
LOCAL_SRC_FILES := $(OPENSSL_ANDROID)/$(TARGET_ARCH_ABI)/libcrypto.a 
include $(PREBUILT_STATIC_LIBRARY)

LOCAL_MODULE := ssl
LOCAL_SRC_FILES := $(OPENSSL_ANDROID)/$(TARGET_ARCH_ABI)/libssl.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_C_INCLUDES := $(OPENSSL_ANDROID)/include \
         ../src

LOCAL_MODULE    := libsuplaclient
LOCAL_STATIC_LIBRARIES := cpufeatures 
LOCAL_LDLIBS := -llog -landroid
LOCAL_SHARED_LIBRARIES = ssl crypto

LOCAL_SRC_FILES := supla-client_jni.c \
    ../../src/cfg.c \
    ../../src/eh.c \
    ../../src/ini.c \
    ../../src/lck.c \
    ../../src/log.c \
    ../../src/proto.c \
    ../../src/safearray.c \
    ../../src/srpc.c \
    ../../src/sthread.c \
    ../../src/supla-client.c \
    ../../src/supla-socket.c \
    ../../src/tools.c \

include $(BUILD_SHARED_LIBRARY)
$(call import-module,android/cpufeatures)
