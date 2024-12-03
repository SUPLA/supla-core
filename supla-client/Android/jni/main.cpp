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

/*
https://docs.oracle.com/javase/7/docs/technotes/guides/jni/spec/types.html

Type Signature | Java Type
Z | boolean
B | byte
C | char
S | short
I | int
J | long
F | float
D | double
 */

#include <android/log.h>
#include <jni.h>
#include <stdlib.h>
#include <string.h>

#include "actions.h"
#include "channel_config.h"
#include "device_config.h"
#include "proto.h"
#include "srpc.h"
#include "supla-client.h"
#include "supla.h"

//  __android_log_write(ANDROID_LOG_DEBUG, log_tag, "XXXX");

#define JNI_FUNCTION_V(jni_function_suffix, supla_client_function)       \
  extern "C" JNIEXPORT jboolean JNICALL                                  \
      Java_org_supla_android_lib_SuplaClient_##jni_function_suffix(      \
          JNIEnv *env, jobject thiz, jlong _asc) {                       \
    void *supla_client = supla_client_ptr(_asc);                         \
    if (supla_client) {                                                  \
      return supla_client_function(supla_client) ? JNI_TRUE : JNI_FALSE; \
    }                                                                    \
    return JNI_FALSE;                                                    \
  }

#define JNI_FUNCTION_I(jni_function_suffix, supla_client_function)           \
  extern "C" JNIEXPORT jboolean JNICALL                                      \
      Java_org_supla_android_lib_SuplaClient_##jni_function_suffix(          \
          JNIEnv *env, jobject thiz, jlong _asc, jint i1) {                  \
    void *supla_client = supla_client_ptr(_asc);                             \
    if (supla_client) {                                                      \
      return supla_client_function(supla_client, i1) ? JNI_TRUE : JNI_FALSE; \
    }                                                                        \
    return JNI_FALSE;                                                        \
  }

#define JNI_FUNCTION_II(jni_function_suffix, supla_client_function)   \
  extern "C" JNIEXPORT jboolean JNICALL                               \
      Java_org_supla_android_lib_SuplaClient_##jni_function_suffix(   \
          JNIEnv *env, jobject thiz, jlong _asc, jint i1, jint i2) {  \
    void *supla_client = supla_client_ptr(_asc);                      \
    if (supla_client) {                                               \
      return supla_client_function(supla_client, i1, i2) ? JNI_TRUE   \
                                                         : JNI_FALSE; \
    }                                                                 \
    return JNI_FALSE;                                                 \
  }

#define JNI_FUNCTION_IString(jni_function_suffix, supla_client_function,       \
                             string_max_size)                                  \
  extern "C" JNIEXPORT jboolean JNICALL                                        \
      Java_org_supla_android_lib_SuplaClient_##jni_function_suffix(            \
          JNIEnv *env, jobject thiz, jlong _asc, jint i1, jstring s1) {        \
    jboolean result = JNI_FALSE;                                               \
    void *supla_client = supla_client_ptr(_asc);                               \
    if (supla_client) {                                                        \
      char str[string_max_size] = {};                                          \
      supla_GetStringUtfChars(env, s1, str, sizeof(str));                      \
      result =                                                                 \
          supla_client_function(supla_client, i1, str) ? JNI_TRUE : JNI_FALSE; \
    }                                                                          \
                                                                               \
    return result;                                                             \
  }

#define JNI_CALLBACK_I(jni_function_cb)                                \
  void supla_cb_##jni_function_cb(void *_suplaclient, void *user_data, \
                                  _supla_int_t i1) {                   \
    ASC_VAR_DECLARATION();                                             \
    ENV_VAR_DECLARATION();                                             \
    if (asc->j_mid_##jni_function_cb == NULL) {                        \
      return;                                                          \
    }                                                                  \
    env->CallVoidMethod(asc->j_obj, asc->j_mid_##jni_function_cb, i1); \
  }

#define JNI_CALLBACK_STRING(jni_function_cb)                           \
  void supla_cb_##jni_function_cb(void *_suplaclient, void *user_data, \
                                  char *str) {                         \
    ASC_VAR_DECLARATION();                                             \
    ENV_VAR_DECLARATION();                                             \
    if (asc->j_mid_##jni_function_cb == NULL) {                        \
      return;                                                          \
    }                                                                  \
    env->CallVoidMethod(asc->j_obj, asc->j_mid_##jni_function_cb,      \
                        str ? new_string_utf(env, str) : NULL);        \
  }

jfieldID supla_client_GetFieldID(JNIEnv *env, jclass c, const char *name,
                                 const char *type) {
  jfieldID fid;

  fid = env->GetFieldID(c, name, type);

  if (fid == NULL) {
    __android_log_print(ANDROID_LOG_ERROR, log_tag,
                        "Unknown field name: %s type: %s", name, type);
  }

  return fid;
}

void supla_android_client(TAndroidSuplaClient *asc, jmethodID mid,
                          jobject obj) {
  ENV_VAR_DECLARATION();

  if (obj) {
    env->CallVoidMethod(asc->j_obj, mid, obj);
  } else {
    env->CallVoidMethod(asc->j_obj, mid);
  }
}

void supla_stringobj2buffer(JNIEnv *env, jobject cfg, jclass jcs,
                            const char *name, char *buff, int size) {
  jfieldID fid = supla_client_GetFieldID(env, jcs, name, "Ljava/lang/String;");
  jstring jstr = (jstring)env->GetObjectField(cfg, fid);

  supla_GetStringUtfChars(env, jstr, buff, size);
}

void supla_bytearrobj2buffer(JNIEnv *env, jobject cfg, jclass jcs,
                             const char *name, char *buff, int size) {
  jfieldID fid = supla_client_GetFieldID(env, jcs, name, "[B");
  jbyteArray barr = (jbyteArray)env->GetObjectField(cfg, fid);

  supla_GetByteArrayElements(env, barr, buff, size);
}

unsigned char supla_shortobj2jshort(JNIEnv *env, jobject in, jshort *out) {
  if (env == NULL || in == NULL || out == NULL) {
    return 0;
  }

  jclass in_cls = env->GetObjectClass(in);

  jmethodID m_get_short_val = env->GetMethodID(in_cls, "shortValue", "()S");
  if (m_get_short_val == NULL) {
    return 0;
  }

  *out = env->CallShortMethod(in, m_get_short_val);

  return 1;
}

jobject supla_jshort2shortobj(JNIEnv *env, jshort in) {
  jclass cls = env->FindClass("java/lang/Short");

  jmethodID m_init = env->GetMethodID(cls, "<init>", "(S)V");
  if (NULL == m_init) {
    return NULL;
  };

  return env->NewObject(cls, m_init, in);
}

jobject supla_jint2integerobj(JNIEnv *env, jint in) {
  jclass cls = env->FindClass("java/lang/Integer");

  jmethodID m_init = env->GetMethodID(cls, "<init>", "(I)V");
  if (NULL == m_init) {
    return NULL;
  };

  return env->NewObject(cls, m_init, in);
}

void supla_cb_on_versionerror(void *_suplaclient, void *user_data, int version,
                              int remote_version_min, int remote_version) {
  ASC_VAR_DECLARATION();
  ENV_VAR_DECLARATION();
  jfieldID fid;

  if (asc->j_mid_on_versionerror) {
    jclass cls = env->FindClass("org/supla/android/lib/SuplaVersionError");

    jmethodID methodID = env->GetMethodID(cls, "<init>", "()V");
    jobject rerr = env->NewObject(cls, methodID);
    jclass crerr = env->GetObjectClass(rerr);

    fid = supla_client_GetFieldID(env, crerr, "Version", "I");
    env->SetIntField(rerr, fid, version);

    fid = supla_client_GetFieldID(env, crerr, "RemoteVersionMin", "I");
    env->SetIntField(rerr, fid, remote_version_min);

    fid = supla_client_GetFieldID(env, crerr, "RemoteVersion", "I");
    env->SetIntField(rerr, fid, remote_version);

    supla_android_client(asc, asc->j_mid_on_versionerror, rerr);
  }
}

void supla_cb_on_connected(void *_suplaclient, void *user_data) {
  ASC_VAR_DECLARATION();

  if (asc->j_mid_on_connected)
    supla_android_client(asc, asc->j_mid_on_connected, NULL);
}

void supla_cb_on_connerror(void *_suplaclient, void *user_data, int code) {
  ASC_VAR_DECLARATION();
  ENV_VAR_DECLARATION();
  jfieldID fid;

  if (asc->j_mid_on_connerror) {
    jclass cls = env->FindClass("org/supla/android/lib/SuplaConnError");

    jmethodID methodID = env->GetMethodID(cls, "<init>", "()V");
    jobject cerr = env->NewObject(cls, methodID);
    jclass ccerr = env->GetObjectClass(cerr);

    fid = supla_client_GetFieldID(env, ccerr, "Code", "I");
    env->SetIntField(cerr, fid, code);

    supla_android_client(asc, asc->j_mid_on_connerror, cerr);
  }
}

void supla_cb_on_disconnected(void *_suplaclient, void *user_data) {
  ASC_VAR_DECLARATION();

  if (asc->j_mid_on_disconnected)
    supla_android_client(asc, asc->j_mid_on_disconnected, NULL);
}

void supla_cb_on_registering(void *_suplaclient, void *user_data) {
  ASC_VAR_DECLARATION();

  if (asc->j_mid_on_registering)
    supla_android_client(asc, asc->j_mid_on_registering, NULL);
}

void supla_cb_on_registered(void *_suplaclient, void *user_data,
                            TSC_SuplaRegisterClientResult_D *result) {
  ASC_VAR_DECLARATION();
  ENV_VAR_DECLARATION();
  jfieldID fid;

  if (asc->j_mid_on_registered) {
    jclass cls = env->FindClass("org/supla/android/lib/SuplaRegisterResult");

    jmethodID methodID = env->GetMethodID(cls, "<init>", "()V");
    jobject reg = env->NewObject(cls, methodID);
    jclass creg = env->GetObjectClass(reg);

    fid = supla_client_GetFieldID(env, creg, "ResultCode", "I");
    env->SetIntField(reg, fid, result->result_code);

    fid = supla_client_GetFieldID(env, creg, "ClientID", "I");
    env->SetIntField(reg, fid, result->ClientID);

    fid = supla_client_GetFieldID(env, creg, "LocationCount", "I");
    env->SetIntField(reg, fid, result->LocationCount);

    fid = supla_client_GetFieldID(env, creg, "ChannelCount", "I");
    env->SetIntField(reg, fid, result->ChannelCount);

    fid = supla_client_GetFieldID(env, creg, "ChannelGroupCount", "I");
    env->SetIntField(reg, fid, result->ChannelGroupCount);

    fid = supla_client_GetFieldID(env, creg, "SceneCount", "I");
    env->SetIntField(reg, fid, result->SceneCount);

    fid = supla_client_GetFieldID(env, creg, "Flags", "I");
    env->SetIntField(reg, fid, result->Flags);

    fid = supla_client_GetFieldID(env, creg, "ActivityTimeout", "I");
    env->SetIntField(reg, fid, result->activity_timeout);

    fid = supla_client_GetFieldID(env, creg, "Version", "I");
    env->SetIntField(reg, fid, result->version);

    fid = supla_client_GetFieldID(env, creg, "VersionMin", "I");
    env->SetIntField(reg, fid, result->version_min);

    supla_android_client(asc, asc->j_mid_on_registered, reg);
  }
}

void supla_cb_on_registererror(void *_suplaclient, void *user_data, int code) {
  ASC_VAR_DECLARATION();
  ENV_VAR_DECLARATION();
  jfieldID fid;

  if (asc->j_mid_on_registererror) {
    jclass cls = env->FindClass("org/supla/android/lib/SuplaRegisterError");

    jmethodID methodID = env->GetMethodID(cls, "<init>", "()V");
    jobject rerr = env->NewObject(cls, methodID);
    jclass crerr = env->GetObjectClass(rerr);

    fid = supla_client_GetFieldID(env, crerr, "ResultCode", "I");
    env->SetIntField(rerr, fid, code);

    supla_android_client(asc, asc->j_mid_on_registererror, rerr);
  }
}

void supla_cb_location_update(void *_suplaclient, void *user_data,
                              TSC_SuplaLocation *location) {
  ASC_VAR_DECLARATION();
  ENV_VAR_DECLARATION();
  jfieldID fid;

  if (asc->j_mid_location_update) {
    jclass cls = env->FindClass("org/supla/android/lib/SuplaLocation");

    jmethodID methodID = env->GetMethodID(cls, "<init>", "()V");
    jobject loc = env->NewObject(cls, methodID);
    jclass cloc = env->GetObjectClass(loc);

    fid = supla_client_GetFieldID(env, cloc, "EOL", "Z");
    env->SetBooleanField(loc, fid, location->EOL == 1 ? JNI_TRUE : JNI_FALSE);

    fid = supla_client_GetFieldID(env, cloc, "Id", "I");
    env->SetIntField(loc, fid, location->Id);

    fid = supla_client_GetFieldID(env, cloc, "Caption", "Ljava/lang/String;");
    env->SetObjectField(loc, fid, new_string_utf(env, location->Caption));

    supla_android_client(asc, asc->j_mid_location_update, loc);
  }
}

jobject supla_channelvalue_to_jobject(void *_suplaclient, void *user_data,
                                      TSuplaChannelValue_B *channel_value) {
  ASC_VAR_DECLARATION(NULL);
  ENV_VAR_DECLARATION(NULL);
  jfieldID fid;

  jclass cls = env->FindClass("org/supla/android/lib/SuplaChannelValue");

  jmethodID methodID = env->GetMethodID(cls, "<init>", "()V");
  jobject val = env->NewObject(cls, methodID);
  jclass cval = env->GetObjectClass(val);

  jbyteArray arr = env->NewByteArray(SUPLA_CHANNELVALUE_SIZE);
  env->SetByteArrayRegion(arr, 0, SUPLA_CHANNELVALUE_SIZE,
                          (const jbyte *)channel_value->value);

  fid = supla_client_GetFieldID(env, cval, "Value", "[B");
  env->SetObjectField(val, fid, arr);

  arr = env->NewByteArray(SUPLA_CHANNELVALUE_SIZE);
  env->SetByteArrayRegion(arr, 0, SUPLA_CHANNELVALUE_SIZE,
                          (const jbyte *)channel_value->sub_value);

  fid = supla_client_GetFieldID(env, cval, "SubValue", "[B");
  env->SetObjectField(val, fid, arr);

  fid = supla_client_GetFieldID(env, cval, "SubValueType", "S");
  env->SetShortField(val, fid, channel_value->sub_value_type);

  return val;
}

jobject supla_online_to_joblect(JNIEnv *env, char online) {
  char enum_name[30] = {};

  switch (online) {
    case 1:
      snprintf(enum_name, sizeof(enum_name), "ONLINE");
      break;
    case 2:
      snprintf(enum_name, sizeof(enum_name), "ONLINE_BUT_NOT_AVAILABLE");
      break;
    default:
      snprintf(enum_name, sizeof(enum_name), "OFFLINE");
      break;
  }

  return supla_NewEnum(env,
                       "org/supla/android/data/source/remote/channel/"
                       "SuplaChannelAvailabilityStatus",
                       enum_name);
}

void supla_cb_channel_update(void *_suplaclient, void *user_data,
                             TSC_SuplaChannel_E *channel) {
  // int a;
  ASC_VAR_DECLARATION();
  ENV_VAR_DECLARATION();
  jfieldID fid;

  // for(a=0;a<SUPLA_CHANNELVALUE_SIZE;a++)
  // __android_log_print(ANDROID_LOG_ERROR, log_tag, "Channel %i, Value[%i]=%i",
  // channel->Id, a, channel->value.value[a]);

  if (asc->j_mid_channel_update) {
    jclass cls = env->FindClass("org/supla/android/lib/SuplaChannel");

    jmethodID methodID = env->GetMethodID(cls, "<init>", "()V");
    jobject ch = env->NewObject(cls, methodID);
    jclass cch = env->GetObjectClass(ch);

    fid = supla_client_GetFieldID(env, cch, "EOL", "Z");
    env->SetBooleanField(ch, fid, channel->EOL == 1 ? JNI_TRUE : JNI_FALSE);

    fid = supla_client_GetFieldID(env, cch, "Id", "I");
    env->SetIntField(ch, fid, channel->Id);

    fid = supla_client_GetFieldID(env, cch, "DeviceID", "I");
    env->SetIntField(ch, fid, channel->DeviceID);

    fid = supla_client_GetFieldID(env, cch, "LocationID", "I");
    env->SetIntField(ch, fid, channel->LocationID);

    fid = supla_client_GetFieldID(env, cch, "Type", "I");
    env->SetIntField(ch, fid, channel->Type);

    fid = supla_client_GetFieldID(env, cch, "Func", "I");
    env->SetIntField(ch, fid, channel->Func);

    fid = supla_client_GetFieldID(env, cch, "ManufacturerID", "S");
    env->SetShortField(ch, fid, channel->ManufacturerID);

    fid = supla_client_GetFieldID(env, cch, "ProductID", "S");
    env->SetShortField(ch, fid, channel->ProductID);

    fid = supla_client_GetFieldID(env, cch, "AvailabilityStatus",
                                  "Lorg/supla/android/data/source/remote/"
                                  "channel/SuplaChannelAvailabilityStatus;");
    env->SetObjectField(ch, fid, supla_online_to_joblect(env, channel->online));

    fid = supla_client_GetFieldID(env, cch, "Value",
                                  "Lorg/supla/android/lib/SuplaChannelValue;");
    jobject chv =
        supla_channelvalue_to_jobject(_suplaclient, user_data, &channel->value);
    env->SetObjectField(ch, fid, chv);

    fid = supla_client_GetFieldID(env, cch, "Caption", "Ljava/lang/String;");
    env->SetObjectField(ch, fid, new_string_utf(env, channel->Caption));

    fid = supla_client_GetFieldID(env, cch, "AltIcon", "I");
    env->SetIntField(ch, fid, channel->AltIcon);

    fid = supla_client_GetFieldID(env, cch, "UserIcon", "I");
    env->SetIntField(ch, fid, channel->UserIcon);

    fid = supla_client_GetFieldID(env, cch, "Flags", "J");
    env->SetLongField(ch, fid, channel->Flags);

    fid = supla_client_GetFieldID(env, cch, "ProtocolVersion", "I");
    env->SetIntField(ch, fid, channel->ProtocolVersion);

    fid = supla_client_GetFieldID(env, cch, "DefaultConfigCRC32", "J");
    env->SetLongField(ch, fid, channel->DefaultConfigCRC32);

    supla_android_client(asc, asc->j_mid_channel_update, ch);
  }
}

void supla_cb_channel_value_update(void *_suplaclient, void *user_data,
                                   TSC_SuplaChannelValue_B *channel_value) {
  ASC_VAR_DECLARATION();
  ENV_VAR_DECLARATION();
  jfieldID fid;

  if (asc->j_mid_channel_value_update) {
    jclass cls =
        env->FindClass("org/supla/android/lib/SuplaChannelValueUpdate");

    jmethodID methodID = env->GetMethodID(cls, "<init>", "()V");
    jobject val = env->NewObject(cls, methodID);
    jclass cval = env->GetObjectClass(val);

    fid = supla_client_GetFieldID(env, cval, "EOL", "Z");
    env->SetBooleanField(val, fid,
                         channel_value->EOL == 1 ? JNI_TRUE : JNI_FALSE);

    fid = supla_client_GetFieldID(env, cval, "Id", "I");
    env->SetIntField(val, fid, channel_value->Id);

    fid = supla_client_GetFieldID(env, cval, "AvailabilityStatus",
                                  "Lorg.supla.android.data.source.remote."
                                  "channel.SuplaChannelAvailabilityStatus;");
    env->SetObjectField(val, fid,
                        supla_online_to_joblect(env, channel_value->online));

    fid = supla_client_GetFieldID(env, cval, "Value",
                                  "Lorg/supla/android/lib/SuplaChannelValue;");

    jobject chv = supla_channelvalue_to_jobject(_suplaclient, user_data,
                                                &channel_value->value);
    env->SetObjectField(val, fid, chv);

    supla_android_client(asc, asc->j_mid_channel_value_update, val);
  }
}

void supla_channel_em_addsummary(TAndroidSuplaClient *asc, JNIEnv *env,
                                 jobject parent, jclass parent_cls,
                                 TElectricityMeter_ExtendedValue_V3 *em_ev,
                                 jint phase) {
  jclass cls = env->FindClass(
      "org/supla/android/lib/SuplaChannelElectricityMeterValue$Summary");
  jmethodID methodID = env->GetMethodID(
      cls, "<init>",
      "(Lorg/supla/android/lib/SuplaChannelElectricityMeterValue;JJJJ)V");

  jlong fae = em_ev->total_forward_active_energy[phase];
  jlong rae = em_ev->total_reverse_active_energy[phase];
  jlong fre = em_ev->total_forward_reactive_energy[phase];
  jlong rre = em_ev->total_reverse_reactive_energy[phase];

  jobject sum_obj = env->NewObject(cls, methodID, parent, fae, rae, fre, rre);

  jmethodID add_summary_mid = env->GetMethodID(
      parent_cls, "addSummary",
      "(ILorg/supla/android/lib/SuplaChannelElectricityMeterValue$Summary;)V");
  phase += 1;
  env->CallVoidMethod(parent, add_summary_mid, phase, sum_obj);
}

void supla_channel_em_addmeasurement(TAndroidSuplaClient *asc, JNIEnv *env,
                                     jobject parent, jclass parent_cls,
                                     TElectricityMeter_ExtendedValue_V3 *em_ev,
                                     jint phase, int midx) {
  jclass cls = env->FindClass(

      "org/supla/android/lib/SuplaChannelElectricityMeterValue$Measurement");
  jmethodID methodID = env->GetMethodID(
      cls, "<init>",
      "(Lorg/supla/android/lib/SuplaChannelElectricityMeterValue;IIIIIIII)V");
  jobject m_obj = env->NewObject(
      cls, methodID, parent, em_ev->m[midx].freq, em_ev->m[midx].voltage[phase],
      em_ev->m[midx].current[phase], em_ev->m[midx].power_active[phase],
      em_ev->m[midx].power_reactive[phase],
      em_ev->m[midx].power_apparent[phase], em_ev->m[midx].power_factor[phase],
      em_ev->m[midx].phase_angle[phase]);

  jmethodID add_m_mid =
      env->GetMethodID(parent_cls, "addMeasurement",
                       "(ILorg/supla/android/lib/"
                       "SuplaChannelElectricityMeterValue$Measurement;)V");
  phase += 1;
  env->CallVoidMethod(parent, add_m_mid, phase, m_obj);
}

jobject supla_channelelectricitymetervalue_to_jobject(
    TAndroidSuplaClient *asc, JNIEnv *env,
    TElectricityMeter_ExtendedValue_V3 *em_ev) {
  int a = 0;
  int b = 0;

  char currency[4];
  memcpy(currency, em_ev->currency, 3);
  currency[sizeof(currency) - 1] = 0;

  jlong fae_b = em_ev->total_forward_active_energy_balanced;
  jlong rae_b = em_ev->total_reverse_active_energy_balanced;
  jint voltage_phase_angle_12 = em_ev->voltage_phase_angle_12;
  jint voltage_phase_angle_13 = em_ev->voltage_phase_angle_13;
  jint phase_sequence = em_ev->phase_sequence;

  jclass cls =
      env->FindClass("org/supla/android/lib/SuplaChannelElectricityMeterValue");
  jmethodID methodID =
      env->GetMethodID(cls, "<init>", "(IIIIIIILjava/lang/String;JJ)V");
  jobject val = env->NewObject(
      cls, methodID, em_ev->measured_values, voltage_phase_angle_12,
      voltage_phase_angle_13, phase_sequence, em_ev->period, em_ev->total_cost,
      em_ev->price_per_unit, new_string_utf(env, currency), fae_b, rae_b);

  for (a = 0; a < 3; a++) {
    supla_channel_em_addsummary(asc, env, val, cls, em_ev, a);
    for (b = 0; b < em_ev->m_count; b++) {
      supla_channel_em_addmeasurement(asc, env, val, cls, em_ev, a, b);
    }
  }

  return val;
}

jobject supla_impulsecountervalue_to_jobject(
    TAndroidSuplaClient *asc, JNIEnv *env,
    TSC_ImpulseCounter_ExtendedValue *ic_ev) {
  jclass cls =
      env->FindClass("org/supla/android/lib/SuplaChannelImpulseCounterValue");
  jmethodID methodID = env->GetMethodID(
      cls, "<init>", "(IJJIILjava/lang/String;Ljava/lang/String;)V");

  char currency[4];
  memcpy(currency, ic_ev->currency, 3);
  currency[sizeof(currency) - 1] = 0;

  ic_ev->custom_unit[sizeof(ic_ev->custom_unit) - 1] = 0;

  jlong counter = ic_ev->counter;
  jlong calculated_value = ic_ev->calculated_value;

  return env->NewObject(cls, methodID, ic_ev->impulses_per_unit, counter,
                        calculated_value, ic_ev->total_cost,
                        ic_ev->price_per_unit, new_string_utf(env, currency),
                        new_string_utf(env, ic_ev->custom_unit));
}

jobject supla_thermostatvalue_to_jobject(TAndroidSuplaClient *asc, JNIEnv *env,
                                         TThermostat_ExtendedValue *th_ev) {
  jclass cls =
      env->FindClass("org/supla/android/lib/SuplaChannelThermostatValue");
  jmethodID methodID = env->GetMethodID(cls, "<init>", "()V");

  jobject m_obj = env->NewObject(cls, methodID);
  int a;

  if (th_ev->Fields & THERMOSTAT_FIELD_MeasuredTemperatures) {
    jmethodID set_measured_temperature_m_mid =
        env->GetMethodID(cls, "setMeasuredTemperature", "(ID)Z");

    for (a = 0; a < sizeof(th_ev->MeasuredTemperature) / sizeof(_supla_int16_t);
         a++) {
      env->CallBooleanMethod(m_obj, set_measured_temperature_m_mid, a,
                             th_ev->MeasuredTemperature[a] / 100.0);
    }
  }

  if (th_ev->Fields & THERMOSTAT_FIELD_PresetTemperatures) {
    jmethodID set_preset_temperature_m_mid =
        env->GetMethodID(cls, "setPresetTemperature", "(ID)Z");

    for (a = 0; a < sizeof(th_ev->PresetTemperature) / sizeof(_supla_int16_t);
         a++) {
      env->CallBooleanMethod(m_obj, set_preset_temperature_m_mid, a,
                             th_ev->PresetTemperature[a] / 100.0);
    }
  }

  if (th_ev->Fields & THERMOSTAT_FIELD_Flags) {
    jmethodID set_flags_m_mid = env->GetMethodID(cls, "setFlags", "(II)Z");

    for (a = 0; a < sizeof(th_ev->Flags) / sizeof(_supla_int16_t); a++) {
      env->CallBooleanMethod(m_obj, set_flags_m_mid, a, th_ev->Flags[a]);
    }
  }

  if (th_ev->Fields & THERMOSTAT_FIELD_Values) {
    jmethodID set_values_m_mid = env->GetMethodID(cls, "setValues", "(II)Z");

    for (a = 0; a < sizeof(th_ev->Values) / sizeof(_supla_int16_t); a++) {
      env->CallBooleanMethod(m_obj, set_values_m_mid, a, th_ev->Values[a]);
    }
  }

  if (th_ev->Fields & THERMOSTAT_FIELD_Time) {
    jmethodID set_time_m_mid = env->GetMethodID(cls, "setTime", "(BBBB)V");

    for (a = 0; a < sizeof(th_ev->Values) / sizeof(_supla_int16_t); a++) {
      env->CallVoidMethod(m_obj, set_time_m_mid, (jbyte)th_ev->Time.sec,
                          (jbyte)th_ev->Time.min, (jbyte)th_ev->Time.hour,
                          (jbyte)th_ev->Time.dayOfWeek);
    }
  }

  if (th_ev->Fields & THERMOSTAT_FIELD_Schedule) {
    jmethodID set_schedule_vtype_m_mid =
        env->GetMethodID(cls, "setScheduleValueType", "(B)V");

    env->CallVoidMethod(m_obj, set_schedule_vtype_m_mid,
                        th_ev->Schedule.ValueType);

    jmethodID set_schedule_value_m_mid =
        env->GetMethodID(cls, "setScheduleValue", "(BBB)V");

    for (a = 0; a < 7; a++) {
      for (int b = 0; b < 24; b++) {
        env->CallVoidMethod(m_obj, set_schedule_value_m_mid, (jbyte)a, (jbyte)b,
                            (jbyte)th_ev->Schedule.HourValue[a][b]);
      }
    }
  }

  return m_obj;
}

jobject supla_channelstate_to_jobject(TAndroidSuplaClient *asc, JNIEnv *env,
                                      TDSC_ChannelState *state) {
  jclass cls = env->FindClass("org/supla/android/lib/SuplaChannelState");
  jmethodID methodID = env->GetMethodID(cls, "<init>", "(IIII[BBBBBBBIIBBII)V");

  jbyteArray mac = env->NewByteArray(6);
  env->SetByteArrayRegion(mac, 0, 6, (const jbyte *)state->MAC);

  return env->NewObject(
      cls, methodID, (jint)state->ChannelID, (jint)state->Fields,
      (jint)state->defaultIconField, (jint)state->IPv4, mac,
      (jbyte)state->BatteryLevel, (jbyte)state->BatteryPowered,
      (jbyte)state->WiFiRSSI, (jbyte)state->WiFiSignalStrength,
      (jbyte)state->BridgeNodeOnline, (jbyte)state->BridgeNodeSignalStrength,
      (jint)state->Uptime, (jint)state->ConnectionUptime,
      (jbyte)state->BatteryHealth, (jbyte)state->LastConnectionResetCause,
      (jint)state->LightSourceLifespan,
      state->Fields & SUPLA_CHANNELSTATE_FIELD_LIGHTSOURCEOPERATINGTIME
          ? (jint)state->LightSourceOperatingTime
          : (jshort)state->LightSourceLifespanLeft);
}

jobject supla_timerstate_to_jobject(TAndroidSuplaClient *asc, JNIEnv *env,
                                    TTimerState_ExtendedValue *state) {
  jclass cls = env->FindClass("org/supla/android/lib/SuplaTimerState");
  jmethodID methodID =
      env->GetMethodID(cls, "<init>", "(J[BILjava/lang/String;)V");

  supla_client_set_str(state->SenderName, &state->SenderNameSize,
                       SUPLA_SENDER_NAME_MAXSIZE);

  jbyteArray arr = env->NewByteArray(SUPLA_CHANNELVALUE_SIZE);
  env->SetByteArrayRegion(arr, 0, SUPLA_CHANNELVALUE_SIZE,
                          (const jbyte *)state->TargetValue);

  jlong countdownEndsAt = (jlong)state->CountdownEndsAt +
                          supla_client_get_time_diff(asc->_supla_client);

  return env->NewObject(cls, methodID, countdownEndsAt, arr, state->SenderID,
                        new_string_utf(env, state->SenderName));
}

void supla_channelextendedvalue_set_object(
    void *_suplaclient, void *user_data,
    TSuplaChannelExtendedValue *channel_extendedvalue, jobject obj) {
  ASC_VAR_DECLARATION();
  ENV_VAR_DECLARATION();

  jfieldID fid;
  jclass cls = env->GetObjectClass(obj);

  TChannelState_ExtendedValue *channel_state = NULL;
  TTimerState_ExtendedValue *timer_state = NULL;

  if (channel_extendedvalue->type == EV_TYPE_ELECTRICITY_METER_MEASUREMENT_V2 ||
      channel_extendedvalue->type == EV_TYPE_ELECTRICITY_METER_MEASUREMENT_V3) {
    TElectricityMeter_ExtendedValue_V3 em_ev = {};

    if (srpc_evtool_extended2emextended_latest(channel_extendedvalue, &em_ev)) {
      fid = supla_client_GetFieldID(
          env, cls, "ElectricityMeterValue",
          "Lorg/supla/android/lib/SuplaChannelElectricityMeterValue;");

      jobject chv =
          supla_channelelectricitymetervalue_to_jobject(asc, env, &em_ev);
      env->SetObjectField(obj, fid, chv);
    }
  } else if (channel_extendedvalue->type ==
             EV_TYPE_IMPULSE_COUNTER_DETAILS_V1) {
    TSC_ImpulseCounter_ExtendedValue ic_ev;
    if (srpc_evtool_v1_extended2icextended(channel_extendedvalue, &ic_ev) ==
        1) {
      fid = supla_client_GetFieldID(
          env, cls, "ImpulseCounterValue",
          "Lorg/supla/android/lib/SuplaChannelImpulseCounterValue;");
      jobject chv = supla_impulsecountervalue_to_jobject(asc, env, &ic_ev);
      env->SetObjectField(obj, fid, chv);
    }
  } else if (channel_extendedvalue->type == EV_TYPE_THERMOSTAT_DETAILS_V1) {
    TThermostat_ExtendedValue th_ev;
    if (srpc_evtool_v1_extended2thermostatextended(channel_extendedvalue,
                                                   &th_ev) == 1) {
      fid = supla_client_GetFieldID(
          env, cls, "ThermostatValue",
          "Lorg/supla/android/lib/SuplaChannelThermostatValue;");

      jobject chv = supla_thermostatvalue_to_jobject(asc, env, &th_ev);

      env->SetObjectField(obj, fid, chv);
    }
  } else if (channel_extendedvalue->type == EV_TYPE_CHANNEL_STATE_V1 &&
             channel_extendedvalue->size ==
                 sizeof(TChannelState_ExtendedValue)) {
    channel_state = (TChannelState_ExtendedValue *)channel_extendedvalue->value;

  } else if ((channel_extendedvalue->type == EV_TYPE_TIMER_STATE_V1 ||
              channel_extendedvalue->type == EV_TYPE_TIMER_STATE_V1_SEC) &&
             channel_extendedvalue->size <= sizeof(TTimerState_ExtendedValue) &&
             channel_extendedvalue->size >= sizeof(TTimerState_ExtendedValue) -
                                                SUPLA_SENDER_NAME_MAXSIZE) {
    timer_state = (TTimerState_ExtendedValue *)channel_extendedvalue->value;

  } else if (channel_extendedvalue->type ==
                 EV_TYPE_CHANNEL_AND_TIMER_STATE_V1 &&
             channel_extendedvalue->size <=
                 sizeof(TChannelAndTimerState_ExtendedValue) &&
             channel_extendedvalue->size >=
                 sizeof(TChannelAndTimerState_ExtendedValue) -
                     SUPLA_SENDER_NAME_MAXSIZE) {
    channel_state =
        &((TChannelAndTimerState_ExtendedValue *)channel_extendedvalue->value)
             ->Channel;
    timer_state =
        &((TChannelAndTimerState_ExtendedValue *)channel_extendedvalue->value)
             ->Timer;
  }

  if (channel_state) {
    fid = supla_client_GetFieldID(env, cls, "ChannelStateValue",
                                  "Lorg/supla/android/lib/SuplaChannelState;");

    // TChannelState_ExtendedValue is equal to TDSC_ChannelState
    jobject channel_state_obj = supla_channelstate_to_jobject(
        asc, env, (TDSC_ChannelState *)channel_state);

    env->SetObjectField(obj, fid, channel_state_obj);
  }

  if (timer_state) {
    fid = supla_client_GetFieldID(env, cls, "TimerStateValue",
                                  "Lorg/supla/android/lib/SuplaTimerState;");

    jobject timer_state_obj = supla_timerstate_to_jobject(
        asc, env, (TTimerState_ExtendedValue *)timer_state);

    env->SetObjectField(obj, fid, timer_state_obj);
  }
}

jobject supla_channelextendedvalue_to_jobject(
    void *_suplaclient, void *user_data,
    TSuplaChannelExtendedValue *channel_extendedvalue) {
  ASC_VAR_DECLARATION(NULL);
  ENV_VAR_DECLARATION(NULL);

  jclass cls =
      env->FindClass("org/supla/android/lib/SuplaChannelExtendedValue");

  jmethodID methodID = env->GetMethodID(cls, "<init>", "()V");
  jobject obj = env->NewObject(cls, methodID);

  int index = 0;
  TSuplaChannelExtendedValue ev = {};

  while (srpc_evtool_value_get(channel_extendedvalue, index, &ev)) {
    index++;
    supla_channelextendedvalue_set_object(_suplaclient, user_data, &ev, obj);
  }

  return obj;
}

void supla_cb_channel_extendedvalue_update(
    void *_suplaclient, void *user_data,
    TSC_SuplaChannelExtendedValue *channel_extendedvalue) {
  ASC_VAR_DECLARATION();
  ENV_VAR_DECLARATION();
  jfieldID fid;

  if (asc->j_mid_channel_extendedvalue_update) {
    jclass cls =
        env->FindClass("org/supla/android/lib/SuplaChannelExtendedValueUpdate");

    jmethodID methodID = env->GetMethodID(cls, "<init>", "()V");
    jobject val = env->NewObject(cls, methodID);
    jclass cval = env->GetObjectClass(val);

    fid = supla_client_GetFieldID(env, cval, "Id", "I");
    env->SetIntField(val, fid, channel_extendedvalue->Id);

    fid = supla_client_GetFieldID(
        env, cval, "Value",
        "Lorg/supla/android/lib/SuplaChannelExtendedValue;");
    jobject chv = supla_channelextendedvalue_to_jobject(
        _suplaclient, user_data, &channel_extendedvalue->value);
    env->SetObjectField(val, fid, chv);

    supla_android_client(asc, asc->j_mid_channel_extendedvalue_update, val);
  }
}

void supla_cb_channelgroup_update(void *_suplaclient, void *user_data,
                                  TSC_SuplaChannelGroup_B *channel_group) {
  ASC_VAR_DECLARATION();
  ENV_VAR_DECLARATION();
  jfieldID fid;

  if (asc->j_mid_channelgroup_update) {
    jclass cls = env->FindClass("org/supla/android/lib/SuplaChannelGroup");

    jmethodID methodID = env->GetMethodID(cls, "<init>", "()V");
    jobject chg = env->NewObject(cls, methodID);
    jclass cchg = env->GetObjectClass(chg);

    fid = supla_client_GetFieldID(env, cchg, "EOL", "Z");
    env->SetBooleanField(chg, fid,
                         channel_group->EOL == 1 ? JNI_TRUE : JNI_FALSE);

    fid = supla_client_GetFieldID(env, cchg, "Id", "I");
    env->SetIntField(chg, fid, channel_group->Id);

    fid = supla_client_GetFieldID(env, cchg, "LocationID", "I");
    env->SetIntField(chg, fid, channel_group->LocationID);

    fid = supla_client_GetFieldID(env, cchg, "Func", "I");
    env->SetIntField(chg, fid, channel_group->Func);

    fid = supla_client_GetFieldID(env, cchg, "AltIcon", "I");
    env->SetIntField(chg, fid, channel_group->AltIcon);

    fid = supla_client_GetFieldID(env, cchg, "UserIcon", "I");
    env->SetIntField(chg, fid, channel_group->UserIcon);

    fid = supla_client_GetFieldID(env, cchg, "Flags", "J");
    env->SetLongField(chg, fid, channel_group->Flags);

    fid = supla_client_GetFieldID(env, cchg, "Caption", "Ljava/lang/String;");
    env->SetObjectField(chg, fid, new_string_utf(env, channel_group->Caption));

    supla_android_client(asc, asc->j_mid_channelgroup_update, chg);
  }
}

void supla_cb_channelgroup_relation_update(
    void *_suplaclient, void *user_data,
    TSC_SuplaChannelGroupRelation *channelgroup_relation) {
  ASC_VAR_DECLARATION();
  ENV_VAR_DECLARATION();
  jfieldID fid;

  if (asc->j_mid_channelgroup_update) {
    jclass cls =
        env->FindClass("org/supla/android/lib/SuplaChannelGroupRelation");

    jmethodID methodID = env->GetMethodID(cls, "<init>", "()V");
    jobject chg = env->NewObject(cls, methodID);
    jclass cchg = env->GetObjectClass(chg);

    fid = supla_client_GetFieldID(env, cchg, "EOL", "Z");
    env->SetBooleanField(
        chg, fid, channelgroup_relation->EOL == 1 ? JNI_TRUE : JNI_FALSE);

    fid = supla_client_GetFieldID(env, cchg, "ChannelGroupID", "I");
    env->SetIntField(chg, fid, channelgroup_relation->ChannelGroupID);

    fid = supla_client_GetFieldID(env, cchg, "ChannelID", "I");
    env->SetIntField(chg, fid, channelgroup_relation->ChannelID);

    supla_android_client(asc, asc->j_mid_channelgroup_relation_update, chg);
  }
}

void supla_cb_channel_relation_update(
    void *_suplaclient, void *user_data,
    TSC_SuplaChannelRelation *channel_relation) {
  ASC_VAR_DECLARATION();
  ENV_VAR_DECLARATION();

  if (asc->j_mid_channel_update) {
    jclass cls = env->FindClass("org/supla/android/lib/SuplaChannelRelation");

    jmethodID methodID = env->GetMethodID(cls, "<init>", "(IISB)V");
    jobject rel = env->NewObject(cls, methodID, channel_relation->Id,
                                 channel_relation->ParentId,
                                 channel_relation->Type, channel_relation->EOL);

    supla_android_client(asc, asc->j_mid_channel_relation_update, rel);
  }
}

void supla_cb_scene_update(void *_suplaclient, void *user_data,
                           TSC_SuplaScene *scene) {
  ASC_VAR_DECLARATION();
  ENV_VAR_DECLARATION();

  if (asc->j_mid_scene_update) {
    jclass cls = env->FindClass("org/supla/android/lib/SuplaScene");
    jmethodID methodID =
        env->GetMethodID(cls, "<init>", "(IIIILjava/lang/String;Z)V");
    jobject scene_obj = env->NewObject(
        cls, methodID, scene->Id, scene->LocationId, scene->AltIcon,
        scene->UserIcon, new_string_utf(env, scene->Caption),
        scene->EOL == 1 ? JNI_TRUE : JNI_FALSE);

    supla_android_client(asc, asc->j_mid_scene_update, scene_obj);
  }
}

void supla_cb_scene_state_update(void *_suplaclient, void *user_data,
                                 TSC_SuplaSceneState *state) {
  ASC_VAR_DECLARATION();
  ENV_VAR_DECLARATION();

  if (asc->j_mid_scene_state_update) {
    jclass cls = env->FindClass("org/supla/android/lib/SuplaSceneState");
    jmethodID methodID =
        env->GetMethodID(cls, "<init>", "(IJJILjava/lang/String;Z)V");

    jlong millisFromStart = state->MillisecondsFromStart;
    jlong millisLeft = state->MillisecondsLeft;

    jobject state_obj = env->NewObject(
        cls, methodID, state->SceneId, millisFromStart, millisLeft,
        state->InitiatorId, new_string_utf(env, state->InitiatorName),
        state->EOL == 1 ? JNI_TRUE : JNI_FALSE);

    supla_android_client(asc, asc->j_mid_scene_state_update, state_obj);
  }
}

void supla_cb_on_oauth_token_request_result(
    void *_suplaclient, void *user_data, TSC_OAuthTokenRequestResult *result) {
  ASC_VAR_DECLARATION();
  ENV_VAR_DECLARATION();

  if (asc->j_mid_on_oauth_token_request_result) {
    jclass cls = env->FindClass("org/supla/android/lib/SuplaOAuthToken");
    jmethodID methodID =
        env->GetMethodID(cls, "<init>", "(IILjava/lang/String;)V");
    jobject token_obj = env->NewObject(
        cls, methodID, result->ResultCode, result->Token.ExpiresIn,
        new_string_utf(env, result->Token.Token));

    supla_android_client(asc, asc->j_mid_on_oauth_token_request_result,
                         token_obj);
  }
}

void supla_cb_on_superuser_authorization_result(void *_suplaclient,
                                                void *user_data,
                                                char authorized,
                                                _supla_int_t code) {
  ASC_VAR_DECLARATION();
  ENV_VAR_DECLARATION();

  if (asc->j_mid_on_superuser_authorization_result) {
    env->CallVoidMethod(asc->j_obj,
                        asc->j_mid_on_superuser_authorization_result,
                        authorized > 0 ? JNI_TRUE : JNI_FALSE, code);
  }
}

void supla_cb_on_device_calcfg_result(void *_suplaclient, void *user_data,
                                      TSC_DeviceCalCfgResult *result) {
  ASC_VAR_DECLARATION();
  ENV_VAR_DECLARATION();

  if (asc->j_mid_on_device_calcfg_result) {
    jbyteArray data = NULL;

    if (result->DataSize > 0) {
      data = env->NewByteArray(result->DataSize);
      env->SetByteArrayRegion(data, 0, result->DataSize,
                              (const jbyte *)result->Data);
    } else {
      data = (jbyteArray)env->NewGlobalRef(NULL);
    }

    env->CallVoidMethod(asc->j_obj, asc->j_mid_on_device_calcfg_result,
                        result->ChannelID, result->Command, result->Result,
                        data);
  }
}

void supla_cb_on_device_calcfg_progress_report(void *_suplaclient,
                                               void *user_data, int ChannelID,
                                               TCalCfg_ProgressReport *result) {
  ASC_VAR_DECLARATION();
  ENV_VAR_DECLARATION();

  if (asc->j_mid_on_device_calcfg_progress_report) {
    env->CallVoidMethod(asc->j_obj, asc->j_mid_on_device_calcfg_progress_report,
                        ChannelID, result->Command, result->Progress);
  }
}

JNI_CALLBACK_STRING(on_device_calcfg_debug_string);

void supla_cb_on_channel_state(void *_suplaclient, void *user_data,
                               TDSC_ChannelState *state) {
  ASC_VAR_DECLARATION();
  ENV_VAR_DECLARATION();

  if (asc->j_mid_on_channel_state == NULL || state == NULL) {
    return;
  }

  jobject channel_state_obj = supla_channelstate_to_jobject(asc, env, state);

  supla_android_client(asc, asc->j_mid_on_channel_state, channel_state_obj);
}

void supla_cb_on_channel_basic_cfg(void *_suplaclient, void *user_data,
                                   TSC_ChannelBasicCfg *cfg) {
  ASC_VAR_DECLARATION();
  ENV_VAR_DECLARATION();

  if (asc->j_mid_on_channel_basic_cfg == NULL || cfg == NULL) {
    return;
  }

  jclass cls = env->FindClass("org/supla/android/lib/SuplaChannelBasicCfg");
  jmethodID methodID = env->GetMethodID(
      cls, "<init>",
      "(Ljava/lang/String;Ljava/lang/String;IIIIIIIIIILjava/lang/String;)V");

  jobject channel_basic_cfg_obj =
      env->NewObject(cls, methodID, new_string_utf(env, cfg->DeviceName),
                     new_string_utf(env, cfg->DeviceSoftVer), cfg->DeviceID,
                     cfg->DeviceFlags, cfg->ManufacturerID, cfg->ProductID,
                     cfg->ID, cfg->Number, cfg->Type, cfg->Func, cfg->FuncList,
                     cfg->ChannelFlags, new_string_utf(env, cfg->Caption));

  supla_android_client(asc, asc->j_mid_on_channel_basic_cfg,
                       channel_basic_cfg_obj);
}

void supla_cb_on_channel_function_set_result(
    void *_suplaclient, void *user_data, TSC_SetChannelFunctionResult *result) {
  ASC_VAR_DECLARATION();
  ENV_VAR_DECLARATION();

  if (asc->j_mid_on_channel_function_set_result == NULL || result == NULL) {
    return;
  }

  env->CallVoidMethod(asc->j_obj, asc->j_mid_on_channel_function_set_result,
                      result->ChannelID, result->Func, result->ResultCode);
}

void supla_cb_on_channel_caption_set_result(void *_suplaclient, void *user_data,
                                            TSCD_SetCaptionResult *result) {
  ASC_VAR_DECLARATION();
  ENV_VAR_DECLARATION();

  if (asc->j_mid_on_channel_caption_set_result == NULL || result == NULL) {
    return;
  }

  env->CallVoidMethod(asc->j_obj, asc->j_mid_on_channel_caption_set_result,
                      result->ID, new_string_utf(env, result->Caption),
                      result->ResultCode);
}

void supla_cb_on_channel_group_caption_set_result(
    void *_suplaclient, void *user_data, TSCD_SetCaptionResult *result) {
  ASC_VAR_DECLARATION();
  ENV_VAR_DECLARATION();

  if (asc->j_mid_on_channel_group_caption_set_result == NULL ||
      result == NULL) {
    return;
  }

  env->CallVoidMethod(
      asc->j_obj, asc->j_mid_on_channel_group_caption_set_result, result->ID,
      new_string_utf(env, result->Caption), result->ResultCode);
}

void supla_cb_on_location_caption_set_result(void *_suplaclient,
                                             void *user_data,
                                             TSCD_SetCaptionResult *result) {
  ASC_VAR_DECLARATION();
  ENV_VAR_DECLARATION();

  if (asc->j_mid_on_location_caption_set_result == NULL || result == NULL) {
    return;
  }

  env->CallVoidMethod(asc->j_obj, asc->j_mid_on_location_caption_set_result,
                      result->ID, new_string_utf(env, result->Caption),
                      result->ResultCode);
}

void supla_cb_on_scene_caption_set_result(void *_suplaclient, void *user_data,
                                          TSCD_SetCaptionResult *result) {
  ASC_VAR_DECLARATION();
  ENV_VAR_DECLARATION();

  if (asc->j_mid_on_scene_caption_set_result == NULL || result == NULL) {
    return;
  }

  env->CallVoidMethod(asc->j_obj, asc->j_mid_on_scene_caption_set_result,
                      result->ID, new_string_utf(env, result->Caption),
                      result->ResultCode);
}

void supla_cb_on_clients_reconnect_result(
    void *_suplaclient, void *user_data,
    TSC_ClientsReconnectRequestResult *result) {
  ASC_VAR_DECLARATION();
  ENV_VAR_DECLARATION();

  if (asc->j_mid_on_clients_reconnect_result == NULL || result == NULL) {
    return;
  }

  env->CallVoidMethod(asc->j_obj, asc->j_mid_on_clients_reconnect_result,
                      result->ResultCode);
}

void supla_cb_on_set_registration_enabled_result(
    void *_suplaclient, void *user_data,
    TSC_SetRegistrationEnabledResult *result) {
  ASC_VAR_DECLARATION();
  ENV_VAR_DECLARATION();

  if (asc->j_mid_on_set_registration_enabled_result == NULL || result == NULL) {
    return;
  }

  env->CallVoidMethod(asc->j_obj, asc->j_mid_on_set_registration_enabled_result,
                      result->ResultCode);
}

void supla_cb_on_event(void *_suplaclient, void *user_data,
                       TSC_SuplaEvent *event) {
  jfieldID fid;
  ASC_VAR_DECLARATION();
  ENV_VAR_DECLARATION();

  if (asc->j_mid_on_event) {
    jclass cls = env->FindClass("org/supla/android/lib/SuplaEvent");

    jmethodID methodID = env->GetMethodID(cls, "<init>", "()V");
    jobject ev = env->NewObject(cls, methodID);
    jclass cev = env->GetObjectClass(ev);

    fid = supla_client_GetFieldID(env, cev, "Event", "I");
    env->SetIntField(ev, fid, event->Event);

    fid = supla_client_GetFieldID(env, cev, "ChannelID", "I");
    env->SetIntField(ev, fid, event->ChannelID);

    jlong durationMS = event->DurationMS;
    fid = supla_client_GetFieldID(env, cev, "DurationMS", "J");
    env->SetLongField(ev, fid, durationMS);

    fid = supla_client_GetFieldID(env, cev, "SenderID", "I");
    env->SetIntField(ev, fid, event->SenderID);

    fid = supla_client_GetFieldID(env, cev, "SenderName", "Ljava/lang/String;");
    env->SetObjectField(ev, fid, new_string_utf(env, event->SenderName));

    supla_android_client(asc, asc->j_mid_on_event, ev);
  }
}

void supla_cb_on_registration_enabled(void *_suplaclient, void *user_data,
                                      TSDC_RegistrationEnabled *reg_enabled) {
  jfieldID fid;
  ASC_VAR_DECLARATION();
  ENV_VAR_DECLARATION();

  if (asc->j_mid_on_registration_enabled) {
    jclass cls =
        env->FindClass("org/supla/android/lib/SuplaRegistrationEnabled");

    jmethodID methodID = env->GetMethodID(cls, "<init>", "()V");
    jobject re = env->NewObject(cls, methodID);
    jclass cre = env->GetObjectClass(re);

    jlong clientTimestamp = reg_enabled->client_timestamp;
    fid = supla_client_GetFieldID(env, cre, "ClientTimestamp", "J");
    env->SetLongField(re, fid, clientTimestamp);

    jlong deviceTimestamp = reg_enabled->iodevice_timestamp;
    fid = supla_client_GetFieldID(env, cre, "IODeviceTimestamp", "J");
    env->SetLongField(re, fid, deviceTimestamp);

    supla_android_client(asc, asc->j_mid_on_registration_enabled, re);
  }
}

void supla_cb_on_min_version_required(void *_suplaclient, void *user_data,
                                      unsigned int call_id,
                                      unsigned char min_version) {
  jfieldID fid;
  ASC_VAR_DECLARATION();
  ENV_VAR_DECLARATION();

  if (asc->j_mid_on_min_version_required) {
    jclass cls =
        env->FindClass("org/supla/android/lib/SuplaMinVersionRequired");

    jmethodID methodID = env->GetMethodID(cls, "<init>", "()V");
    jobject mv = env->NewObject(cls, methodID);
    jclass cmv = env->GetObjectClass(mv);

    jlong callId = call_id;
    fid = supla_client_GetFieldID(env, cmv, "CallType", "J");
    env->SetLongField(mv, fid, callId);

    fid = supla_client_GetFieldID(env, cmv, "MinVersion", "I");
    env->SetIntField(mv, fid, min_version);

    supla_android_client(asc, asc->j_mid_on_min_version_required, mv);
  }
}

JNI_CALLBACK_I(on_zwave_reset_and_clear_result);
JNI_CALLBACK_I(on_zwave_set_wake_up_time_result);

void supla_cb_on_zwave_remove_node_result(void *_suplaclient, void *user_data,
                                          _supla_int_t result,
                                          unsigned char node_id) {
  ASC_VAR_DECLARATION();
  ENV_VAR_DECLARATION();

  if (asc->j_mid_on_zwave_remove_node_result == NULL) {
    return;
  }

  env->CallVoidMethod(asc->j_obj, asc->j_mid_on_zwave_remove_node_result,
                      result, (jshort)node_id);
}

jobject supla_zwave_node_to_jobject(TAndroidSuplaClient *asc, JNIEnv *env,
                                    TCalCfg_ZWave_Node *node) {
  jclass cls = env->FindClass("org/supla/android/lib/ZWaveNode");
  jmethodID methodID = env->GetMethodID(
      cls, "<init>", "(SSILjava/lang/Integer;Ljava/lang/String;)V");

  jobject channelID = node->Flags & ZWAVE_NODE_FLAG_CHANNEL_ASSIGNED
                          ? supla_jint2integerobj(env, node->ChannelID)
                          : 0;

  return env->NewObject(cls, methodID, (jshort)node->Id,
                        (jshort)node->ScreenType, (jint)node->Flags, channelID,
                        new_string_utf(env, node->Name));
}

jobject supla_zwave_wake_up_settings_report_to_jobject(
    TAndroidSuplaClient *asc, JNIEnv *env,
    TCalCfg_ZWave_WakeupSettingsReport *report) {
  if (report == NULL) {
    return NULL;
  }
  jclass cls = env->FindClass("org/supla/android/lib/ZWaveWakeUpSettings");
  jmethodID methodID = env->GetMethodID(cls, "<init>", "(IIII)V");

  return env->NewObject(cls, methodID, (jint)report->MinimumSec,
                        (jint)report->MaximumSec, (jint)report->ValueSec,
                        (jint)report->IntervalStepSec);
}

void supla_cb_on_zwave_add_node_result(void *_suplaclient, void *user_data,
                                       _supla_int_t result,
                                       TCalCfg_ZWave_Node *node) {
  ASC_VAR_DECLARATION();
  ENV_VAR_DECLARATION();

  if (asc->j_mid_on_zwave_add_node_result == NULL) {
    return;
  }

  env->CallVoidMethod(
      asc->j_obj, asc->j_mid_on_zwave_add_node_result, result,
      node == NULL ? NULL : supla_zwave_node_to_jobject(asc, env, node));
}

void supla_cb_on_zwave_get_node_list_result(void *_suplaclient, void *user_data,
                                            _supla_int_t result,
                                            TCalCfg_ZWave_Node *node) {
  ASC_VAR_DECLARATION();
  ENV_VAR_DECLARATION();

  if (asc->j_mid_on_zwave_get_node_list_result == NULL) {
    return;
  }

  env->CallVoidMethod(
      asc->j_obj, asc->j_mid_on_zwave_get_node_list_result, result,
      node == NULL ? NULL : supla_zwave_node_to_jobject(asc, env, node));
}

void supla_cb_on_zwave_get_assigned_node_id_result(void *_suplaclient,
                                                   void *user_data,
                                                   _supla_int_t result,
                                                   unsigned char node_id) {
  ASC_VAR_DECLARATION();
  ENV_VAR_DECLARATION();

  if (asc->j_mid_on_zwave_get_assigned_node_id_result == NULL) {
    return;
  }

  env->CallVoidMethod(asc->j_obj,
                      asc->j_mid_on_zwave_get_assigned_node_id_result, result,
                      (jshort)node_id);
}

void supla_cb_on_zwave_wake_up_settings_report(
    void *_suplaclient, void *user_data, _supla_int_t result,
    TCalCfg_ZWave_WakeupSettingsReport *report) {
  ASC_VAR_DECLARATION();
  ENV_VAR_DECLARATION();

  if (asc->j_mid_on_zwave_wake_up_settings_report == NULL) {
    return;
  }

  jobject settings =
      supla_zwave_wake_up_settings_report_to_jobject(asc, env, report);

  env->CallVoidMethod(asc->j_obj, asc->j_mid_on_zwave_wake_up_settings_report,
                      result, settings);
}

void supla_cb_on_zwave_assign_node_id_result(void *_suplaclient,
                                             void *user_data,
                                             _supla_int_t result,
                                             unsigned char node_id) {
  ASC_VAR_DECLARATION();
  ENV_VAR_DECLARATION();

  if (asc->j_mid_on_zwave_assign_node_id_result == NULL) {
    return;
  }

  env->CallVoidMethod(asc->j_obj, asc->j_mid_on_zwave_assign_node_id_result,
                      result, (jshort)node_id);
}

extern "C" JNIEXPORT void JNICALL
Java_org_supla_android_lib_SuplaClient_cfgInit(JNIEnv *env, jobject thiz,
                                               jobject cfg) {
  TSuplaClientCfg sclient_cfg;
  jfieldID fid;

  jclass jcs = env->GetObjectClass(cfg);

  if (jcs) {
    supla_client_cfginit(&sclient_cfg);

    fid = supla_client_GetFieldID(env, jcs, "ssl_enabled", "Z");
    env->SetBooleanField(cfg, fid,
                         sclient_cfg.ssl_enabled == 1 ? JNI_TRUE : JNI_FALSE);

    fid = supla_client_GetFieldID(env, jcs, "ssl_port", "I");
    env->SetIntField(cfg, fid, sclient_cfg.ssl_port);

    fid = supla_client_GetFieldID(env, jcs, "tcp_port", "I");
    env->SetIntField(cfg, fid, sclient_cfg.tcp_port);

    char host[] = "";
    if (sclient_cfg.host == NULL) sclient_cfg.host = host;

    fid = supla_client_GetFieldID(env, jcs, "Host", "Ljava/lang/String;");
    env->SetObjectField(cfg, fid, new_string_utf(env, sclient_cfg.host));

    fid = supla_client_GetFieldID(env, jcs, "SoftVer", "Ljava/lang/String;");
    env->SetObjectField(cfg, fid, new_string_utf(env, sclient_cfg.SoftVer));

    fid =
        supla_client_GetFieldID(env, jcs, "AccessIDpwd", "Ljava/lang/String;");
    env->SetObjectField(cfg, fid, new_string_utf(env, sclient_cfg.AccessIDpwd));

    fid = supla_client_GetFieldID(env, jcs, "AccessID", "I");
    env->SetIntField(cfg, fid, sclient_cfg.AccessID);

    fid = supla_client_GetFieldID(env, jcs, "protocol_version", "I");
    env->SetIntField(cfg, fid, 0);

    fid = supla_client_GetFieldID(env, jcs, "Email", "Ljava/lang/String;");
    env->SetObjectField(cfg, fid, new_string_utf(env, sclient_cfg.Email));

    fid = supla_client_GetFieldID(env, jcs, "Password", "Ljava/lang/String;");
    env->SetObjectField(cfg, fid, new_string_utf(env, sclient_cfg.Password));

    fid = supla_client_GetFieldID(env, jcs, "Name", "Ljava/lang/String;");
    env->SetObjectField(cfg, fid, new_string_utf(env, sclient_cfg.Name));

    jbyteArray arr = env->NewByteArray(SUPLA_GUID_SIZE);
    env->SetByteArrayRegion(arr, 0, SUPLA_GUID_SIZE,
                            (const jbyte *)sclient_cfg.clientGUID);

    fid = supla_client_GetFieldID(env, jcs, "clientGUID", "[B");
    env->SetObjectField(cfg, fid, arr);

    arr = env->NewByteArray(SUPLA_AUTHKEY_SIZE);
    env->SetByteArrayRegion(arr, 0, SUPLA_AUTHKEY_SIZE,
                            (const jbyte *)sclient_cfg.AuthKey);

    fid = supla_client_GetFieldID(env, jcs, "AuthKey", "[B");
    env->SetObjectField(cfg, fid, arr);
  };
}

extern "C" JNIEXPORT jlong JNICALL
Java_org_supla_android_lib_SuplaClient_scInit(JNIEnv *env, jobject thiz,
                                              jobject cfg) {
  TSuplaClientCfg sclient_cfg;
  jfieldID fid;

  jclass jcs = env->GetObjectClass(cfg);

  if (jcs) {
    supla_client_cfginit(&sclient_cfg);

    fid = supla_client_GetFieldID(env, jcs, "ssl_enabled", "Z");
    sclient_cfg.ssl_enabled =
        env->GetBooleanField(cfg, fid) == JNI_TRUE ? 1 : 0;

    fid = supla_client_GetFieldID(env, jcs, "ssl_port", "I");
    sclient_cfg.ssl_port = env->GetIntField(cfg, fid);

    fid = supla_client_GetFieldID(env, jcs, "tcp_port", "I");
    sclient_cfg.tcp_port = env->GetIntField(cfg, fid);

    fid = supla_client_GetFieldID(env, jcs, "Host", "Ljava/lang/String;");
    jstring js_host = (jstring)env->GetObjectField(cfg, fid);
    sclient_cfg.host = (char *)env->GetStringUTFChars(js_host, 0);

    supla_stringobj2buffer(env, cfg, jcs, "SoftVer", sclient_cfg.SoftVer,
                           SUPLA_SOFTVER_MAXSIZE);

    supla_stringobj2buffer(env, cfg, jcs, "AccessIDpwd",
                           sclient_cfg.AccessIDpwd, SUPLA_ACCESSID_PWD_MAXSIZE);

    fid = supla_client_GetFieldID(env, jcs, "AccessID", "I");
    sclient_cfg.AccessID = env->GetIntField(cfg, fid);

    fid = supla_client_GetFieldID(env, jcs, "protocol_version", "I");
    sclient_cfg.protocol_version = env->GetIntField(cfg, fid);

    if (sclient_cfg.protocol_version < SUPLA_PROTO_VERSION_MIN ||
        sclient_cfg.protocol_version > SUPLA_PROTO_VERSION) {
      sclient_cfg.protocol_version = SUPLA_PROTO_VERSION;
    }

    supla_stringobj2buffer(env, cfg, jcs, "Email", sclient_cfg.Email,
                           SUPLA_EMAIL_MAXSIZE);

    supla_stringobj2buffer(env, cfg, jcs, "Password", sclient_cfg.Password,
                           SUPLA_PASSWORD_MAXSIZE);

    supla_stringobj2buffer(env, cfg, jcs, "Name", sclient_cfg.Name,
                           SUPLA_CLIENT_NAME_MAXSIZE);

    supla_bytearrobj2buffer(env, cfg, jcs, "clientGUID", sclient_cfg.clientGUID,
                            SUPLA_GUID_SIZE);

    supla_bytearrobj2buffer(env, cfg, jcs, "AuthKey", sclient_cfg.AuthKey,
                            SUPLA_AUTHKEY_SIZE);

    TAndroidSuplaClient *_asc =
        (TAndroidSuplaClient *)malloc(sizeof(TAndroidSuplaClient));
    memset(_asc, 0, sizeof(TAndroidSuplaClient));

    _asc->j_obj = env->NewGlobalRef(thiz);

    jclass oclass = env->GetObjectClass(thiz);

    _asc->j_mid_on_versionerror =
        env->GetMethodID(oclass, "onVersionError",
                         "(Lorg/supla/android/lib/SuplaVersionError;)V");
    _asc->j_mid_on_connected = env->GetMethodID(oclass, "onConnected", "()V");
    _asc->j_mid_on_connerror = env->GetMethodID(
        oclass, "onConnError", "(Lorg/supla/android/lib/SuplaConnError;)V");
    _asc->j_mid_on_disconnected =
        env->GetMethodID(oclass, "onDisconnected", "()V");
    _asc->j_mid_on_registering =
        env->GetMethodID(oclass, "onRegistering", "()V");
    _asc->j_mid_on_registered =
        env->GetMethodID(oclass, "onRegistered",
                         "(Lorg/supla/android/lib/SuplaRegisterResult;)V");
    _asc->j_mid_on_registererror =
        env->GetMethodID(oclass, "onRegisterError",
                         "(Lorg/supla/android/lib/SuplaRegisterError;)V");
    _asc->j_mid_location_update = env->GetMethodID(
        oclass, "locationUpdate", "(Lorg/supla/android/lib/SuplaLocation;)V");
    _asc->j_mid_channel_update = env->GetMethodID(
        oclass, "channelUpdate", "(Lorg/supla/android/lib/SuplaChannel;)V");
    _asc->j_mid_channel_value_update =
        env->GetMethodID(oclass, "channelValueUpdate",
                         "(Lorg/supla/android/lib/SuplaChannelValueUpdate;)V");
    _asc->j_mid_channel_extendedvalue_update = env->GetMethodID(
        oclass, "channelExtendedValueUpdate",
        "(Lorg/supla/android/lib/SuplaChannelExtendedValueUpdate;)V");
    _asc->j_mid_on_event = env->GetMethodID(
        oclass, "onEvent", "(Lorg/supla/android/lib/SuplaEvent;)V");
    _asc->j_mid_on_registration_enabled =
        env->GetMethodID(oclass, "onRegistrationEnabled",
                         "(Lorg/supla/android/lib/SuplaRegistrationEnabled;)V");
    _asc->j_mid_on_min_version_required =
        env->GetMethodID(oclass, "onMinVersionRequired",
                         "(Lorg/supla/android/lib/SuplaMinVersionRequired;)V");
    _asc->j_mid_channel_relation_update =
        env->GetMethodID(oclass, "channelRelationUpdate",
                         "(Lorg/supla/android/lib/SuplaChannelRelation;)V");
    _asc->j_mid_channelgroup_update =
        env->GetMethodID(oclass, "channelGroupUpdate",
                         "(Lorg/supla/android/lib/SuplaChannelGroup;)V");
    _asc->j_mid_channelgroup_relation_update = env->GetMethodID(
        oclass, "channelGroupRelationUpdate",
        "(Lorg/supla/android/lib/SuplaChannelGroupRelation;)V");
    _asc->j_mid_scene_update = env->GetMethodID(
        oclass, "sceneUpdate", "(Lorg/supla/android/lib/SuplaScene;)V");
    _asc->j_mid_scene_state_update =
        env->GetMethodID(oclass, "sceneStateUpdate",
                         "(Lorg/supla/android/lib/SuplaSceneState;)V");
    _asc->j_mid_on_oauth_token_request_result =
        env->GetMethodID(oclass, "onOAuthTokenRequestResult",
                         "(Lorg/supla/android/lib/SuplaOAuthToken;)V");
    _asc->j_mid_on_superuser_authorization_result =
        env->GetMethodID(oclass, "onSuperUserAuthorizationResult", "(ZI)V");
    _asc->j_mid_on_device_calcfg_result =
        env->GetMethodID(oclass, "onDeviceCalCfgResult", "(III[B)V");
    _asc->j_mid_on_device_calcfg_progress_report =
        env->GetMethodID(oclass, "onDeviceCalCfgProgressReport", "(IIS)V");
    _asc->j_mid_on_device_calcfg_debug_string = env->GetMethodID(
        oclass, "onDeviceCalCfgDebugString", "(Ljava/lang/String;)V");
    _asc->j_mid_on_channel_state =
        env->GetMethodID(oclass, "onChannelState",
                         "(Lorg/supla/android/lib/SuplaChannelState;)V");
    _asc->j_mid_on_channel_basic_cfg =
        env->GetMethodID(oclass, "onChannelBasicCfg",
                         "(Lorg/supla/android/lib/SuplaChannelBasicCfg;)V");
    _asc->j_mid_on_channel_function_set_result =
        env->GetMethodID(oclass, "onChannelFunctionSetResult", "(III)V");
    _asc->j_mid_on_channel_caption_set_result = env->GetMethodID(
        oclass, "onChannelCaptionSetResult", "(ILjava/lang/String;I)V");
    _asc->j_mid_on_channel_group_caption_set_result = env->GetMethodID(
        oclass, "onChannelGroupCaptionSetResult", "(ILjava/lang/String;I)V");
    _asc->j_mid_on_location_caption_set_result = env->GetMethodID(
        oclass, "onLocationCaptionSetResult", "(ILjava/lang/String;I)V");
    _asc->j_mid_on_scene_caption_set_result = env->GetMethodID(
        oclass, "onSceneCaptionSetResult", "(ILjava/lang/String;I)V");
    _asc->j_mid_on_clients_reconnect_result =
        env->GetMethodID(oclass, "onClientsReconnectResult", "(I)V");
    _asc->j_mid_on_set_registration_enabled_result =
        env->GetMethodID(oclass, "onSetRegistrationEnabledResult", "(I)V");
    _asc->j_mid_on_zwave_reset_and_clear_result =
        env->GetMethodID(oclass, "onZWaveResetAndClearResult", "(I)V");
    _asc->j_mid_on_zwave_add_node_result =
        env->GetMethodID(oclass, "onZWaveAddNodeResult",
                         "(ILorg/supla/android/lib/ZWaveNode;)V");
    _asc->j_mid_on_zwave_remove_node_result =
        env->GetMethodID(oclass, "onZWaveRemoveNodeResult", "(IS)V");
    _asc->j_mid_on_zwave_get_node_list_result =
        env->GetMethodID(oclass, "onZWaveGetNodeListResult",
                         "(ILorg/supla/android/lib/ZWaveNode;)V");
    _asc->j_mid_on_zwave_get_assigned_node_id_result =
        env->GetMethodID(oclass, "onZWaveGetAssignedNodeIdResult", "(IS)V");
    _asc->j_mid_on_zwave_wake_up_settings_report =
        env->GetMethodID(oclass, "onZWaveWakeUpSettingsReport",
                         "(ILorg/supla/android/lib/ZWaveWakeUpSettings;)V");
    _asc->j_mid_on_zwave_assign_node_id_result =
        env->GetMethodID(oclass, "onZWaveAssignNodeIdResult", "(IS)V");
    _asc->j_mid_on_zwave_set_wake_up_time_result =
        env->GetMethodID(oclass, "onZWaveSetWakeUpTimeResult", "(I)V");

    sclient_cfg.user_data = _asc;
    sclient_cfg.cb_on_versionerror = supla_cb_on_versionerror;
    sclient_cfg.cb_on_connected = supla_cb_on_connected;
    sclient_cfg.cb_on_connerror = supla_cb_on_connerror;
    sclient_cfg.cb_on_disconnected = supla_cb_on_disconnected;
    sclient_cfg.cb_on_registering = supla_cb_on_registering;
    sclient_cfg.cb_on_registered = supla_cb_on_registered;
    sclient_cfg.cb_on_registererror = supla_cb_on_registererror;
    sclient_cfg.cb_location_update = supla_cb_location_update;
    sclient_cfg.cb_channel_update = supla_cb_channel_update;
    sclient_cfg.cb_channel_value_update = supla_cb_channel_value_update;
    sclient_cfg.cb_channel_extendedvalue_update =
        supla_cb_channel_extendedvalue_update;
    sclient_cfg.cb_on_event = supla_cb_on_event;
    sclient_cfg.cb_on_registration_enabled = supla_cb_on_registration_enabled;
    sclient_cfg.cb_on_min_version_required = supla_cb_on_min_version_required;
    sclient_cfg.cb_channel_relation_update = supla_cb_channel_relation_update;
    sclient_cfg.cb_channelgroup_update = supla_cb_channelgroup_update;
    sclient_cfg.cb_channelgroup_relation_update =
        supla_cb_channelgroup_relation_update;
    sclient_cfg.cb_scene_update = supla_cb_scene_update;
    sclient_cfg.cb_scene_state_update = supla_cb_scene_state_update;
    sclient_cfg.cb_on_oauth_token_request_result =
        supla_cb_on_oauth_token_request_result;
    sclient_cfg.cb_on_superuser_authorization_result =
        supla_cb_on_superuser_authorization_result;
    sclient_cfg.cb_on_device_calcfg_result = supla_cb_on_device_calcfg_result;
    sclient_cfg.cb_on_device_calcfg_progress_report =
        supla_cb_on_device_calcfg_progress_report;
    sclient_cfg.cb_on_device_calcfg_debug_string =
        supla_cb_on_device_calcfg_debug_string;
    sclient_cfg.cb_on_device_channel_state = supla_cb_on_channel_state;
    sclient_cfg.cb_on_channel_basic_cfg = supla_cb_on_channel_basic_cfg;
    sclient_cfg.cb_on_channel_function_set_result =
        supla_cb_on_channel_function_set_result;
    sclient_cfg.cb_on_channel_caption_set_result =
        supla_cb_on_channel_caption_set_result;
    sclient_cfg.cb_on_channel_group_caption_set_result =
        supla_cb_on_channel_group_caption_set_result;
    sclient_cfg.cb_on_location_caption_set_result =
        supla_cb_on_location_caption_set_result;
    sclient_cfg.cb_on_scene_caption_set_result =
        supla_cb_on_scene_caption_set_result;
    sclient_cfg.cb_on_clients_reconnect_request_result =
        supla_cb_on_clients_reconnect_result;
    sclient_cfg.cb_on_set_registration_enabled_result =
        supla_cb_on_set_registration_enabled_result;
    sclient_cfg.cb_on_zwave_reset_and_clear_result =
        supla_cb_on_zwave_reset_and_clear_result;
    sclient_cfg.cb_on_zwave_add_node_result = supla_cb_on_zwave_add_node_result;
    sclient_cfg.cb_on_zwave_remove_node_result =
        supla_cb_on_zwave_remove_node_result;
    sclient_cfg.cb_on_zwave_get_node_list_result =
        supla_cb_on_zwave_get_node_list_result;
    sclient_cfg.cb_on_zwave_get_assigned_node_id_result =
        supla_cb_on_zwave_get_assigned_node_id_result;
    sclient_cfg.cb_on_zwave_assign_node_id_result =
        supla_cb_on_zwave_assign_node_id_result;
    sclient_cfg.cb_on_zwave_wake_up_settings_report =
        supla_cb_on_zwave_wake_up_settings_report;
    sclient_cfg.cb_on_zwave_set_wake_up_time_result =
        supla_cb_on_zwave_set_wake_up_time_result;

    supla_channel_config_init(env, oclass, _asc, &sclient_cfg);
    supla_device_config_init(env, oclass, _asc, &sclient_cfg);

    _asc->_supla_client = supla_client_init(&sclient_cfg);

    if (sclient_cfg.host) {
      env->ReleaseStringUTFChars(js_host, sclient_cfg.host);
    }

    if (_asc->_supla_client) {
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

extern "C" JNIEXPORT void JNICALL Java_org_supla_android_lib_SuplaClient_scFree(
    JNIEnv *env, jobject thiz, jlong _asc) {
#ifdef _LP64
  TAndroidSuplaClient *asc = (TAndroidSuplaClient *)_asc;
#else
  TAndroidSuplaClient *asc = (TAndroidSuplaClient *)(int)_asc;
#endif

  if (asc) {
    if (asc->_supla_client) {
      supla_client_free(asc->_supla_client);
      asc->_supla_client = NULL;
    }

    if (asc->j_obj) {
      env->DeleteGlobalRef(asc->j_obj);
      asc->j_obj = NULL;
    }
  }
};

extern "C" JNIEXPORT jint JNICALL
Java_org_supla_android_lib_SuplaClient_scGetId(JNIEnv *env, jobject thiz,
                                               jlong _asc) {
  void *supla_client = supla_client_ptr(_asc);

  if (supla_client) return supla_client_get_id(supla_client);

  return 0;
};

extern "C" JNIEXPORT jboolean JNICALL
Java_org_supla_android_lib_SuplaClient_scConnect(JNIEnv *env, jobject thiz,
                                                 jlong _asc,
                                                 jint conn_timeout_ms) {
  void *supla_client = supla_client_ptr(_asc);

  if (supla_client)
    return supla_client_connect(supla_client, conn_timeout_ms) == 1 ? JNI_TRUE
                                                                    : JNI_FALSE;

  return JNI_FALSE;
};

extern "C" JNIEXPORT jboolean JNICALL
Java_org_supla_android_lib_SuplaClient_scConnected(JNIEnv *env, jobject thiz,
                                                   jlong _asc) {
  void *supla_client = supla_client_ptr(_asc);

  if (supla_client)
    return supla_client_connected(supla_client) == 1 ? JNI_TRUE : JNI_FALSE;

  return JNI_FALSE;
};

extern "C" JNIEXPORT jboolean JNICALL
Java_org_supla_android_lib_SuplaClient_scRegistered(JNIEnv *env, jobject thiz,
                                                    jlong _asc) {
  void *supla_client = supla_client_ptr(_asc);

  if (supla_client)
    return supla_client_registered(supla_client) == 1 ? JNI_TRUE : JNI_FALSE;

  return JNI_FALSE;
};

extern "C" JNIEXPORT void JNICALL
Java_org_supla_android_lib_SuplaClient_scDisconnect(JNIEnv *env, jobject thiz,
                                                    jlong _asc) {
  void *supla_client = supla_client_ptr(_asc);

  if (supla_client) supla_client_disconnect(supla_client);
};

extern "C" JNIEXPORT jboolean JNICALL
Java_org_supla_android_lib_SuplaClient_scIterate(JNIEnv *env, jobject thiz,
                                                 jlong _asc, jint wait_usec) {
  void *supla_client = supla_client_ptr(_asc);

  return supla_client && supla_client_iterate(supla_client, wait_usec) == 1
             ? JNI_TRUE
             : JNI_FALSE;
};

extern "C" JNIEXPORT jboolean JNICALL
Java_org_supla_android_lib_SuplaClient_scOpen(JNIEnv *env, jobject thiz,
                                              jlong _asc, jint id, jint group,
                                              jint open) {
  void *supla_client = supla_client_ptr(_asc);

  if (supla_client)
    return supla_client_open(supla_client, id, group > 0 ? 1 : 0, open) == 1
               ? JNI_TRUE
               : JNI_FALSE;

  return JNI_FALSE;
};

extern "C" JNIEXPORT jboolean JNICALL
Java_org_supla_android_lib_SuplaClient_scTimerArm(JNIEnv *env, jobject thiz,
                                                  jlong _asc, jint channelId,
                                                  jint on, jint durationMS) {
  void *supla_client = supla_client_ptr(_asc);

  if (supla_client)
    return supla_client_timer_arm(supla_client, channelId, on, durationMS) == 1
               ? JNI_TRUE
               : JNI_FALSE;

  return JNI_FALSE;
};

extern "C" JNIEXPORT jboolean JNICALL
Java_org_supla_android_lib_SuplaClient_scSetRGBW(
    JNIEnv *env, jobject thiz, jlong _asc, jint id, jint group, jint color,
    int color_brightness, int brightness, jint turn_onoff) {
  void *supla_client = supla_client_ptr(_asc);

  if (supla_client) {
    if (color_brightness > 255 || color_brightness < 0) color_brightness = 0;

    if (brightness > 255 || brightness < 0) brightness = 0;

    return supla_client_set_rgbw(supla_client, id, group > 0 ? 1 : 0, color,
                                 color_brightness, brightness, turn_onoff) == 1
               ? JNI_TRUE
               : JNI_FALSE;
  }

  return JNI_FALSE;
};

extern "C" JNIEXPORT jboolean JNICALL
Java_org_supla_android_lib_SuplaClient_scGetRegistrationEnabled(JNIEnv *env,
                                                                jobject thiz,
                                                                jlong _asc) {
  void *supla_client = supla_client_ptr(_asc);

  if (supla_client) {
    supla_client_get_registration_enabled(supla_client);
    return JNI_TRUE;
  }

  return JNI_FALSE;
};

extern "C" JNIEXPORT jint JNICALL
Java_org_supla_android_lib_SuplaClient_scGetProtoVersion(JNIEnv *env,
                                                         jobject thiz,
                                                         jlong _asc) {
  void *supla_client = supla_client_ptr(_asc);

  if (supla_client) {
    return supla_client_get_proto_version(supla_client);
  }

  return 0;
};

extern "C" JNIEXPORT jint JNICALL
Java_org_supla_android_lib_SuplaClient_scGetMaxProtoVersion(JNIEnv *env,
                                                            jobject thiz,
                                                            jlong _asc) {
  return SUPLA_PROTO_VERSION;
};

extern "C" JNIEXPORT jboolean JNICALL
Java_org_supla_android_lib_SuplaClient_scOAuthTokenRequest(JNIEnv *env,
                                                           jobject thiz,
                                                           jlong _asc) {
  void *supla_client = supla_client_ptr(_asc);

  if (supla_client)
    return supla_client_oauth_token_request(supla_client) == 1 ? JNI_TRUE
                                                               : JNI_FALSE;

  return JNI_FALSE;
};

extern "C" JNIEXPORT jboolean JNICALL
Java_org_supla_android_lib_SuplaClient_scDeviceCalCfgRequest(
    JNIEnv *env, jobject thiz, jlong _asc, jint id, jint group, jint command,
    jint dataType, jbyteArray data) {
  void *supla_client = supla_client_ptr(_asc);

  if (supla_client) {
    TCS_DeviceCalCfgRequest_B request;
    memset(&request, 0, sizeof(TCS_DeviceCalCfgRequest_B));

    jsize len = env->IsSameObject(data, NULL) ? 0 : env->GetArrayLength(data);

    if (len > 0 && len <= SUPLA_CALCFG_DATA_MAXSIZE) {
      jbyte *d = env->GetByteArrayElements(data, 0);
      for (int a = 0; a < len; a++) {
        request.Data[a] = d[a];
      }
      request.DataSize = len;
      env->ReleaseByteArrayElements(data, d, 0);
    }

    request.Id = id;
    request.Target = group > 0 ? SUPLA_TARGET_GROUP : SUPLA_TARGET_CHANNEL;
    request.Command = command;
    request.DataType = dataType;
    return supla_client_device_calcfg_request(supla_client, &request) == 1
               ? JNI_TRUE
               : JNI_FALSE;
  }

  return JNI_FALSE;
};

extern "C" JNIEXPORT jboolean JNICALL
Java_org_supla_android_lib_SuplaClient_scThermostatScheduleCfgRequest(
    JNIEnv *env, jobject thiz, jlong _asc, jint id, jint group, jobject cfg) {
  void *supla_client = supla_client_ptr(_asc);
  jboolean result = JNI_FALSE;

  if (supla_client) {
    jclass cls =
        env->FindClass("org/supla/android/lib/SuplaThermostatScheduleCfg");

    jmethodID get_group_m_mid = env->GetMethodID(cls, "getGroupCount", "()I");
    jmethodID get_group_value_type_m_mid =
        env->GetMethodID(cls, "getGroupHourValueType", "(I)I");
    jmethodID get_group_week_days_m_mid =
        env->GetMethodID(cls, "getGroupWeekDays", "(I)I");
    jmethodID get_group_hour_value_m_mid =
        env->GetMethodID(cls, "getGroupHourValue", "(IS)B");

    jint groupCount = env->CallIntMethod(cfg, get_group_m_mid);

    if (groupCount > 0) {
      TCS_DeviceCalCfgRequest_B request;
      memset(&request, 0, sizeof(TCS_DeviceCalCfgRequest_B));

      request.Id = id;
      request.Target = group > 0 ? SUPLA_TARGET_GROUP : SUPLA_TARGET_CHANNEL;
      request.Command = SUPLA_THERMOSTAT_CMD_SET_SCHEDULE;
      request.DataSize = sizeof(TThermostat_ScheduleCfg);

      TThermostat_ScheduleCfg *scfg = (TThermostat_ScheduleCfg *)request.Data;

      unsigned char failed = 0;
      unsigned char success = 0;

      int n = 0;
      for (int a = 0; a < groupCount; a++) {
        jint groupValueType =
            env->CallIntMethod(cfg, get_group_value_type_m_mid, a);
        jint groupWeekDays =
            env->CallIntMethod(cfg, get_group_week_days_m_mid, a);

        for (short h = 0; h < 24; h++) {
          scfg->Group[n].HourValue[h] =
              env->CallByteMethod(cfg, get_group_hour_value_m_mid, a, h);
        }

        scfg->Group[n].ValueType = (unsigned char)groupValueType;
        scfg->Group[n].WeekDays = (unsigned char)groupWeekDays;

        n++;
        if (n > 3 || a == groupCount - 1) {
          if (supla_client_device_calcfg_request(supla_client, &request)) {
            success = 1;
          } else {
            failed = 1;
          }
          n = 0;
          memset(scfg, 0, sizeof(TThermostat_ScheduleCfg));
        }
      }

      return success && !failed ? JNI_TRUE : JNI_FALSE;
    }
  }

  return result;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_org_supla_android_lib_SuplaClient_scSuperUserAuthorizationRequest(
    JNIEnv *env, jobject thiz, jlong _asc, jstring email, jstring password) {
  jboolean result = JNI_FALSE;

  void *supla_client = supla_client_ptr(_asc);

  if (supla_client) {
    char eml[SUPLA_EMAIL_MAXSIZE] = {};
    char pwd[SUPLA_PASSWORD_MAXSIZE] = {};

    supla_GetStringUtfChars(env, email, eml, sizeof(eml));
    supla_GetStringUtfChars(env, password, pwd, sizeof(pwd));

    result = supla_client_superuser_authorization_request(supla_client, eml,
                                                          pwd) == 1
                 ? JNI_TRUE
                 : JNI_FALSE;
  }

  return result;
}

JNI_FUNCTION_V(scGetSuperUserAuthorizationResult,
               supla_client_get_superuser_authorization_result);

JNI_FUNCTION_I(scGetChannelState, supla_client_get_channel_state);

JNI_FUNCTION_I(scGetChannelBasicCfg, supla_client_get_channel_basic_cfg);

JNI_FUNCTION_II(scSetChannelFunction, supla_client_set_channel_function);

JNI_FUNCTION_IString(scSetChannelCaption, supla_client_set_channel_caption,
                     SUPLA_CHANNEL_CAPTION_MAXSIZE);

JNI_FUNCTION_IString(scSetChannelGroupCaption,
                     supla_client_set_channel_group_caption,
                     SUPLA_CHANNEL_GROUP_CAPTION_MAXSIZE);

JNI_FUNCTION_IString(scSetLocationCaption, supla_client_set_location_caption,
                     SUPLA_LOCATION_CAPTION_MAXSIZE);

JNI_FUNCTION_IString(scSetSceneCaption, supla_client_set_scene_caption,
                     SUPLA_SCENE_CAPTION_MAXSIZE);

JNI_FUNCTION_V(scReconnectAllClients, supla_client_reconnect_all_clients);

JNI_FUNCTION_II(scSetRegistrationEnabled,
                supla_client_set_registration_enabled);

JNI_FUNCTION_I(scReconnectDevice, supla_client_reconnect_device);

JNI_FUNCTION_I(scZWaveConfigModeActive, supla_client_zwave_config_mode_active);

JNI_FUNCTION_I(scZWaveResetAndClear, supla_client_zwave_reset_and_clear);

JNI_FUNCTION_I(scZWaveAddNode, supla_client_zwave_add_node);

JNI_FUNCTION_I(scZWaveRemoveNode, supla_client_zwave_remove_node);

JNI_FUNCTION_I(scZWaveGetNodeList, supla_client_zwave_get_node_list);

JNI_FUNCTION_I(scZWaveGetAssignedNodeId,
               supla_client_zwave_get_assigned_node_id);

extern "C" JNIEXPORT jboolean JNICALL
Java_org_supla_android_lib_SuplaClient_scZWaveAssignNodeId(
    JNIEnv *env, jobject thiz, jlong _asc, jint channel_id, jshort node_id) {
  void *supla_client = supla_client_ptr(_asc);
  if (supla_client) {
    unsigned char id = 0;

    if (node_id < 0) {
      node_id = 0;
    } else if (node_id > 255) {
      node_id = 255;
    }
    id = node_id;

    return supla_client_zwave_assign_node_id(supla_client, channel_id, id)
               ? JNI_TRUE
               : JNI_FALSE;
  }
  return JNI_FALSE;
}

JNI_FUNCTION_I(scZWaveGetWakeUpSettings,
               supla_client_zwave_get_wake_up_settings);

JNI_FUNCTION_II(scZWaveSetWakeUpTime, supla_client_zwave_set_wake_up_time);

JNI_FUNCTION_I(scDeviceCalCfgCancelAllCommands,
               supla_client_device_calcfg_cancel_all_commands);

extern "C" JNIEXPORT jboolean JNICALL
Java_org_supla_android_lib_SuplaClient_scSetLightsourceLifespan(
    JNIEnv *env, jobject thiz, jlong _asc, jint channel_id,
    jboolean reset_counter, jboolean set_time, jint lifespan) {
  void *supla_client = supla_client_ptr(_asc);
  if (supla_client) {
    if (lifespan < 0) {
      lifespan = 0;
    } else if (lifespan > 65535) {
      lifespan = 65535;
    }

    return supla_client_set_lightsource_lifespan(
               supla_client, channel_id, reset_counter, set_time, lifespan)
               ? JNI_TRUE
               : JNI_FALSE;
  }

  return JNI_FALSE;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_org_supla_android_lib_SuplaClient_scSetDfgTransparency(
    JNIEnv *env, jobject thiz, jlong _asc, jint channel_id, jshort mask,
    jshort active_bits) {
  void *supla_client = supla_client_ptr(_asc);
  if (supla_client) {
    TCSD_Digiglass_NewValue new_value;
    memset(&new_value, 0, sizeof(TCSD_Digiglass_NewValue));

    return supla_client_set_dgf_transparency(supla_client, channel_id, mask,
                                             active_bits)
               ? JNI_TRUE
               : JNI_FALSE;
  }

  return JNI_FALSE;
}
