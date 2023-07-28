/*
 Copyright (C) AC SOFTWARE SP. Z O.O.

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "supla.h"

#include <android/log.h>
#include <string.h>

#include "proto.h"

char log_tag[] = "LibSuplaClient";

void *supla_client_ptr(jlong _asc) {
#ifdef _LP64
  TAndroidSuplaClient *asc = (TAndroidSuplaClient *)_asc;
#else
  TAndroidSuplaClient *asc = (TAndroidSuplaClient *)(int)_asc;
#endif

  if (asc) return asc->_supla_client;

  return NULL;
}

void supla_GetStringUtfChars(JNIEnv *env, jstring jstr, char *buff,
                             size_t size) {
  const char *str = env->GetStringUTFChars(jstr, 0);

  if (str) {
    size_t len = strnlen(str, size);
    if (len > size - 1) {
      len = size - 1;
    }

    memcpy(buff, str, len);
    buff[size - 1] = 0;

    env->ReleaseStringUTFChars(jstr, str);
  } else {
    memset(buff, 0, size);
  }
}

void supla_GetByteArrayElements(JNIEnv *env, jbyteArray barr, char *buff,
                                size_t size) {
  if (barr && size == env->GetArrayLength(barr)) {
    jbyte *data = (jbyte *)env->GetByteArrayElements(barr, NULL);
    if (data) {
      memcpy(buff, data, size);
      env->ReleaseByteArrayElements(barr, data, 0);
    }
  }
}

jobject supla_CallObjectMethod(JNIEnv *env, jclass cls, jobject obj,
                               const char *method_name, const char *type) {
  jmethodID method_id = env->GetMethodID(cls, method_name, type);

  return env->CallObjectMethod(obj, method_id);
}

jlong supla_CallLongMethod(JNIEnv *env, jclass cls, jobject obj,
                           const char *method_name) {
  jmethodID method_id = env->GetMethodID(cls, method_name, "()J");

  return env->CallLongMethod(obj, method_id);
}

bool supla_CallLongObjectMethod(JNIEnv *env, jclass cls, jobject obj,
                                const char *method_name, jlong *result) {
  jobject l =
      supla_CallObjectMethod(env, cls, obj, method_name, "()Ljava/lang/Long;");
  if (l) {
    jclass l_cls = env->FindClass("java/lang/Long");

    *result = supla_CallLongMethod(env, l_cls, l, "longValue");
    return true;
  }

  return false;
}

jint supla_CallIntMethod(JNIEnv *env, jclass cls, jobject obj,
                         const char *method_name) {
  jmethodID method_id = env->GetMethodID(cls, method_name, "()I");

  return env->CallIntMethod(obj, method_id);
}

bool supla_CallIntObjectMethod(JNIEnv *env, jclass cls, jobject obj,
                               const char *method_name, jint *result) {
  jobject i = supla_CallObjectMethod(env, cls, obj, method_name,
                                     "()Ljava/lang/Integer;");
  if (i) {
    jclass i_cls = env->FindClass("java/lang/Integer");

    *result = supla_CallIntMethod(env, i_cls, i, "intValue");
    return true;
  }

  return false;
}

jshort supla_CallShortMethod(JNIEnv *env, jclass cls, jobject obj,
                             const char *method_name) {
  jmethodID method_id = env->GetMethodID(cls, method_name, "()S");

  return env->CallShortMethod(obj, method_id);
}

jboolean supla_CallBooleanMethod(JNIEnv *env, jclass cls, jobject obj,
                                 const char *method_name) {
  jmethodID method_id = env->GetMethodID(cls, method_name, "()Z");

  return env->CallBooleanMethod(obj, method_id);
}

jdouble supla_CallDoubleMethod(JNIEnv *env, jclass cls, jobject obj,
                               const char *method_name) {
  jmethodID method_id = env->GetMethodID(cls, method_name, "()D");

  return env->CallDoubleMethod(obj, method_id);
}

bool supla_CallDoubleObjectMethod(JNIEnv *env, jclass cls, jobject obj,
                                  const char *method_name, jdouble *result) {
  jobject d = supla_CallObjectMethod(env, cls, obj, method_name,
                                     "()Ljava/lang/Double;");
  if (d) {
    jclass d_cls = env->FindClass("java/lang/Double");

    *result = supla_CallDoubleMethod(env, d_cls, d, "doubleValue");
    return true;
  }

  return false;
}

jobject supla_NewInt(JNIEnv *env, jint value) {
  jclass cls = env->FindClass("java/lang/Integer");
  jmethodID midInit = env->GetMethodID(cls, "<init>", "(I)V");
  return env->NewObject(cls, midInit, value);
}

jobject supla_NewDouble(JNIEnv *env, jdouble value) {
  jclass cls = env->FindClass("java/lang/Double");
  jmethodID midInit = env->GetMethodID(cls, "<init>", "(D)V");
  return env->NewObject(cls, midInit, value);
}
