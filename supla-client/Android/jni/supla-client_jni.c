/*
 ============================================================================
 Name        : supla_client-jni.c
 Author      : Przemyslaw Zygmunt p.zygmunt@acsoftware.pl [AC SOFTWARE]
 Version     : 1.0
 Copyright   : GPLv2
 ============================================================================
*/

#include <jni.h>
#include <android/log.h>
#include <string.h>

#include "proto.h"
#include "supla-client.h"

static char log_tag[] = "LibSuplaClient";
//  __android_log_write(ANDROID_LOG_DEBUG, log_tag, "XXXX");

typedef struct {

    void *_supla_client;
    
    jobject j_obj;
    
    jmethodID j_mid_on_versionerror;
    jmethodID j_mid_on_connected;
    jmethodID j_mid_on_disconnected;
    jmethodID j_mid_on_registering;
    jmethodID j_mid_on_registered;
    jmethodID j_mid_on_registererror;
    jmethodID j_mid_location_update;
    jmethodID j_mid_channel_update;
    jmethodID j_mid_channel_value_update;
    jmethodID j_mid_on_event;
    
}TAndroidSuplaClient;

static JavaVM *java_vm;

jint JNI_OnLoad(JavaVM *vm, void *reserved)
{
    java_vm = vm;
    
    JNIEnv* env;
    if ((*vm)->GetEnv(vm, (void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        __android_log_write(ANDROID_LOG_DEBUG, log_tag, "GetEnv failed.");
        return -1;
    }
    
    return JNI_VERSION_1_6;
};

JNIEnv *supla_client_get_env(TAndroidSuplaClient *asc) {
    
    JNIEnv *env = NULL;
    int getEnvStat;
    
    if ( asc && asc->j_obj ) {
        
        getEnvStat = (*java_vm)->GetEnv(java_vm, (void**) &env, JNI_VERSION_1_6);
        
        if (getEnvStat == JNI_OK) {
            return env;
        }
    }

    
    return NULL;
}

void supla_android_client(TAndroidSuplaClient *asc, jmethodID mid, jobject obj) {
    
    JNIEnv *env = supla_client_get_env(asc);
    if ( env ) {
        
        if ( obj )
            (*env)->CallVoidMethod(env, asc->j_obj, mid, obj);
        else
            (*env)->CallVoidMethod(env, asc->j_obj, mid);
    }
    
}

void supla_android_client_cb_on_versionerror(void *_suplaclient, void *user_data, int version, int remote_version_min, int remote_version) {
    
}

void supla_android_client_cb_on_connected(void *_suplaclient, void *user_data) {

    
    TAndroidSuplaClient *asc = (TAndroidSuplaClient*)user_data;
    
    if ( asc && asc->j_mid_on_connected )
        supla_android_client(asc, asc->j_mid_on_connected, NULL);
    
}

void supla_android_client_cb_on_disconnected(void *_suplaclient, void *user_data) {
    
    
    TAndroidSuplaClient *asc = (TAndroidSuplaClient*)user_data;
    
    if ( asc && asc->j_mid_on_disconnected )
        supla_android_client(asc, asc->j_mid_on_disconnected, NULL);
    
}

void supla_android_client_cb_on_registering(void *_suplaclient, void *user_data) {
    
    TAndroidSuplaClient *asc = (TAndroidSuplaClient*)user_data;
    
    if ( asc && asc->j_mid_on_registering )
        supla_android_client(asc, asc->j_mid_on_registering, NULL);
    
}

void supla_android_client_cb_on_registered(void *_suplaclient, void *user_data, TSC_SuplaRegisterClientResult *result) {
    
}

void supla_android_client_cb_on_registererror(void *_suplaclient, void *user_data, int code) {
    
    jfieldID fid;
    TAndroidSuplaClient *asc = (TAndroidSuplaClient*)user_data;
    JNIEnv* env = supla_client_get_env(asc);

    
    if ( env && asc && asc->j_mid_on_registererror ) {
         jclass cls = (*env)->FindClass(env, "Lorg/supla/android/SuplaRegisterError;");
        
         jmethodID methodID = (*env)->GetMethodID(env, cls, "<init>", "()V");
         jobject rerr = (*env)->NewObject(env,cls, methodID);
        
         fid = (*env)->GetFieldID(env, rerr, "code", "I");
         (*env)->SetIntField(env, rerr, fid, code);
        
         supla_android_client(asc, asc->j_mid_on_registererror, rerr);
    }
    
}

void supla_android_client_cb_location_update(void *_suplaclient, void *user_data, TSC_SuplaLocation *location) {
    
}

void supla_android_client_cb_channel_update(void *_suplaclient, void *user_data, TSC_SuplaChannel *channel) {
    
}

void supla_android_client_cb_channel_value_update(void *_suplaclient, void *user_data, TSC_SuplaChannelValue *channel_value) {
    
}

void supla_android_client_cb_on_event(void *_suplaclient, void *user_data, TSC_SuplaEvent *event) {
    
    
}



void supla_android_client_jstring_to_buffer(JNIEnv* env, jobject cfg, jclass jcs, const char *name, char *buff, int size) {
    
    jfieldID fid = (*env)->GetFieldID(env, jcs, name, "Ljava/lang/String;");
    jstring js_str = (*env)->GetObjectField(env, cfg, fid);
    const char *str = (*env)->GetStringUTFChars(env, js_str, 0);
    
    if ( str ) {
        memcpy(buff, str, strlen(str) >= size ? size-1 : strlen(str));
        (*env)->ReleaseStringUTFChars(env, js_str, str);
    }
}

void* supla_client_ptr(jlong _asc) {
    
#ifdef _LP64
    TAndroidSuplaClient *asc = (void*)_asc;
#else
    TAndroidSuplaClient *asc = (void*)(int)_asc;
#endif
    
    if ( asc )
        return asc->_supla_client;
    
    return NULL;
}

JNIEXPORT void JNICALL
Java_org_supla_android_SuplaClient_CfgInit(JNIEnv* env, jobject thiz, jobject cfg) {

    TSuplaClientCfg sclient_cfg;
    jfieldID fid;
    
    jclass jcs = (*env)->GetObjectClass(env, cfg);
    
    if ( jcs ) {
        
        supla_client_cfginit(&sclient_cfg);
        
        fid = (*env)->GetFieldID(env, jcs, "ssl_enabled", "Z");
        (*env)->SetBooleanField(env, cfg, fid, sclient_cfg.ssl_enabled == 1 ? JNI_TRUE : JNI_FALSE);
        
        fid = (*env)->GetFieldID(env, jcs, "ssl_port", "I");
        (*env)->SetIntField(env, cfg, fid, sclient_cfg.ssl_port);
        
        fid = (*env)->GetFieldID(env, jcs, "tcp_port", "I");
        (*env)->SetIntField(env, cfg, fid, sclient_cfg.tcp_port);
        
        if ( sclient_cfg.host == NULL )
            sclient_cfg.host = "";
        
        fid = (*env)->GetFieldID(env, jcs, "Host", "Ljava/lang/String;");
        (*env)->SetObjectField(env, cfg, fid, (*env)->NewStringUTF(env, sclient_cfg.host));
        
        fid = (*env)->GetFieldID(env, jcs, "SoftVer", "Ljava/lang/String;");
        (*env)->SetObjectField(env, cfg, fid, (*env)->NewStringUTF(env, sclient_cfg.SoftVer));
        
        fid = (*env)->GetFieldID(env, jcs, "AccessIDpwd", "Ljava/lang/String;");
        (*env)->SetObjectField(env, cfg, fid, (*env)->NewStringUTF(env, sclient_cfg.AccessIDpwd));
        
        fid = (*env)->GetFieldID(env, jcs, "AccessID", "I");
        (*env)->SetIntField(env, cfg, fid, sclient_cfg.AccessID);
        
        fid = (*env)->GetFieldID(env, jcs, "Name", "Ljava/lang/String;");
        (*env)->SetObjectField(env, cfg, fid, (*env)->NewStringUTF(env, sclient_cfg.Name));
        
        jbyteArray arr = (*env)->NewByteArray(env, SUPLA_GUID_SIZE);
        (*env)->SetByteArrayRegion (env, arr, 0, SUPLA_GUID_SIZE, sclient_cfg.clientGUID);
        
        fid = (*env)->GetFieldID(env, jcs, "clientGUID", "[B");
        (*env)->SetObjectField(env, cfg, fid, arr);
        
    };
    
    
}

JNIEXPORT jlong JNICALL
Java_org_supla_android_SuplaClient_scInit(JNIEnv* env, jobject thiz, jobject cfg) {


    TSuplaClientCfg sclient_cfg;
    jfieldID fid;
    
    jclass jcs = (*env)->GetObjectClass(env, cfg);
    
    if ( jcs ) {
        supla_client_cfginit(&sclient_cfg);
        
        fid = (*env)->GetFieldID(env, jcs, "ssl_enabled", "Z");
        sclient_cfg.ssl_enabled = (*env)->GetBooleanField(env, cfg, fid) == JNI_TRUE ? 1 : 0;
        
        fid = (*env)->GetFieldID(env, jcs, "ssl_port", "I");
        sclient_cfg.ssl_port = (*env)->GetIntField(env, cfg, fid);
        
        fid = (*env)->GetFieldID(env, jcs, "tcp_port", "I");
        sclient_cfg.tcp_port = (*env)->GetIntField(env, cfg, fid);
        
        fid = (*env)->GetFieldID(env, jcs, "Host", "Ljava/lang/String;");
        jstring js_host = (*env)->GetObjectField(env, cfg, fid);
        sclient_cfg.host = (char *)(*env)->GetStringUTFChars(env, js_host, 0);
        
        supla_android_client_jstring_to_buffer(env, cfg, jcs, "SoftVer", sclient_cfg.SoftVer, SUPLA_SOFTVER_MAXSIZE);
        
        supla_android_client_jstring_to_buffer(env, cfg, jcs, "AccessIDpwd", sclient_cfg.AccessIDpwd, SUPLA_ACCESSID_PWD_MAXSIZE);
        
        fid = (*env)->GetFieldID(env, jcs, "AccessID", "I");
        sclient_cfg.AccessID = (*env)->GetIntField(env, cfg, fid);
        
        supla_android_client_jstring_to_buffer(env, cfg, jcs, "Name", sclient_cfg.Name, SUPLA_CLIENT_NAME_MAXSIZE);
        
        fid = (*env)->GetFieldID(env, jcs, "clientGUID", "[B");
        jbyteArray barr = (*env)->GetObjectField(env, cfg, fid);
        
        if ( SUPLA_GUID_SIZE == (*env)->GetArrayLength(env, barr) ) {
            
            jbyte *data = (jbyte *)(*env)->GetByteArrayElements(env, barr, NULL);
            if ( data ) {
                memcpy(sclient_cfg.clientGUID, data, SUPLA_GUID_SIZE);
                (*env)->ReleaseByteArrayElements(env, barr, data, 0 );
            }
        }
        
        TAndroidSuplaClient *_asc = malloc(sizeof(TAndroidSuplaClient));
        memset(_asc, 0, sizeof(TAndroidSuplaClient));
        
        _asc->j_obj = (*env)->NewGlobalRef(env, thiz);
        
        jclass oclass = (*env)->GetObjectClass(env, thiz);
        
        _asc->j_mid_on_versionerror = (*env)->GetMethodID(env, oclass, "onVersionError", "()V");
        _asc->j_mid_on_connected = (*env)->GetMethodID(env, oclass, "onConnected", "()V");
        _asc->j_mid_on_disconnected = (*env)->GetMethodID(env, oclass, "onDisconnected", "()V");
        _asc->j_mid_on_registering = (*env)->GetMethodID(env, oclass, "onRegistering", "()V");
        _asc->j_mid_on_registered = (*env)->GetMethodID(env, oclass, "onRegistered", "()V");
        _asc->j_mid_on_registererror = (*env)->GetMethodID(env, oclass, "onRegisterError", "(Lorg/supla/android/SuplaRegisterError;)V");
        _asc->j_mid_location_update = (*env)->GetMethodID(env, oclass, "LocationUpdate", "()V");
        _asc->j_mid_channel_update = (*env)->GetMethodID(env, oclass, "ChannelUpdate", "()V");
        _asc->j_mid_channel_value_update = (*env)->GetMethodID(env, oclass, "ChannelValueUpdate", "()V");
        _asc->j_mid_on_event = (*env)->GetMethodID(env, oclass, "onEvent", "()V");
        
    
        sclient_cfg.user_data = _asc;
        sclient_cfg.cb_on_versionerror = supla_android_client_cb_on_versionerror;
        sclient_cfg.cb_on_connected = supla_android_client_cb_on_connected;
        sclient_cfg.cb_on_disconnected = supla_android_client_cb_on_disconnected;
        sclient_cfg.cb_on_registering = supla_android_client_cb_on_registering;
        sclient_cfg.cb_on_registered = supla_android_client_cb_on_registered;
        sclient_cfg.cb_on_registererror = supla_android_client_cb_on_registererror;
        sclient_cfg.cb_location_update = supla_android_client_cb_location_update;
        sclient_cfg.cb_channel_update = supla_android_client_cb_channel_update;
        sclient_cfg.cb_channel_value_update = supla_android_client_cb_channel_value_update;
        sclient_cfg.cb_on_event = supla_android_client_cb_on_event;
        
        _asc->_supla_client = supla_client_init(&sclient_cfg);
        
        if ( sclient_cfg.host ) {
            (*env)->ReleaseStringUTFChars(env, js_host, sclient_cfg.host);
        }
        
        if ( _asc->_supla_client ) {

            #ifdef _LP64
            return (jlong)_asc;
            #else
            return (int)_asc;
            #endif
            
        } else {
            free(_asc);
        }
    }

    return 0;
}

JNIEXPORT void JNICALL
Java_org_supla_android_SuplaClient_scFree(JNIEnv* env, jobject thiz, jlong _asc) {
    
#ifdef _LP64
    TAndroidSuplaClient *asc = (void*)_asc;
#else
    TAndroidSuplaClient *asc = (void*)(int)_asc;
#endif
    
    if ( asc ) {
        
        if ( asc->_supla_client ) {
            supla_client_free(asc->_supla_client);
            asc->_supla_client = NULL;
        }
        
        if ( asc->j_obj ) {
            (*env)->DeleteGlobalRef(env, asc->j_obj);
            asc->j_obj = NULL;
        }
    }
    
};

JNIEXPORT jint JNICALL
Java_org_supla_android_SuplaClient_scGetId(JNIEnv* env, jobject thiz, jlong _asc) {
  
    void *supla_client = supla_client_ptr(_asc);
    
    if ( supla_client )
        return supla_client_get_id(supla_client);
    
    return 0;
};


JNIEXPORT jboolean JNICALL
Java_org_supla_android_SuplaClient_scConnect(JNIEnv* env, jobject thiz, jlong _asc) {
    
    void *supla_client = supla_client_ptr(_asc);
    
    if ( supla_client )
        return supla_client_connect(supla_client) == 1 ? JNI_TRUE : JNI_FALSE;
    
    return JNI_FALSE;
};

JNIEXPORT jboolean JNICALL
Java_org_supla_android_SuplaClient_scConnected(JNIEnv* env, jobject thiz, jlong _asc) {
    
    void *supla_client = supla_client_ptr(_asc);
    
    if ( supla_client )
        return supla_client_connected(supla_client) == 1 ? JNI_TRUE : JNI_FALSE;
    
    return JNI_FALSE;
};

JNIEXPORT jboolean JNICALL
Java_org_supla_android_SuplaClient_scRegistered(JNIEnv* env, jobject thiz, jlong _asc) {
    
    void *supla_client = supla_client_ptr(_asc);
    
    if ( supla_client )
        return supla_client_registered(supla_client) == 1 ? JNI_TRUE : JNI_FALSE;
    
    return JNI_FALSE;
};

JNIEXPORT void JNICALL
Java_org_supla_android_SuplaClient_scDisconnect(JNIEnv* env, jobject thiz, jlong _asc) {
    
    void *supla_client = supla_client_ptr(_asc);
    
    if ( supla_client )
        supla_client_disconnect(supla_client);
    
};

JNIEXPORT void JNICALL
Java_org_supla_android_SuplaClient_scIterate(JNIEnv* env, jobject thiz, jlong _asc, jint wait_usec) {
    
    void *supla_client = supla_client_ptr(_asc);
    
    if ( supla_client )
        supla_client_iterate(supla_client, wait_usec);
    
};

JNIEXPORT jboolean JNICALL
Java_org_supla_android_SuplaClient_scOpen(JNIEnv* env, jobject thiz, jlong _asc, jint channelid, jint open) {
    
    void *supla_client = supla_client_ptr(_asc);
    
    if ( supla_client )
        return supla_client_open(supla_client, channelid, open) == 1 ? JNI_TRUE : JNI_FALSE;
    
    return JNI_FALSE;
};