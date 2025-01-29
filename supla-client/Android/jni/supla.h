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

#ifndef SUPLA_H_
#define SUPLA_H_

#include <jni.h>

extern char log_tag[];
extern JavaVM *java_vm;

#define ASC_VAR_DECLARATION(...)                               \
  TAndroidSuplaClient *asc = (TAndroidSuplaClient *)user_data; \
  if (asc == NULL) {                                           \
    return __VA_ARGS__;                                        \
  }

#define ENV_VAR_DECLARATION(...)           \
  JNIEnv *env = supla_client_get_env(asc); \
  if (env == NULL) {                       \
    return __VA_ARGS__;                    \
  }

typedef struct {
  void *_supla_client;

  jobject j_obj;

  jmethodID j_mid_on_versionerror;
  jmethodID j_mid_on_connected;
  jmethodID j_mid_on_connerror;
  jmethodID j_mid_on_disconnected;
  jmethodID j_mid_on_registering;
  jmethodID j_mid_on_registered;
  jmethodID j_mid_on_registererror;
  jmethodID j_mid_location_update;
  jmethodID j_mid_channel_update;
  jmethodID j_mid_channel_value_update;
  jmethodID j_mid_channel_extendedvalue_update;
  jmethodID j_mid_on_event;
  jmethodID j_mid_on_registration_enabled;
  jmethodID j_mid_on_min_version_required;
  jmethodID j_mid_channel_relation_update;
  jmethodID j_mid_channelgroup_update;
  jmethodID j_mid_channelgroup_relation_update;
  jmethodID j_mid_scene_update;
  jmethodID j_mid_scene_state_update;
  jmethodID j_mid_on_oauth_token_request_result;
  jmethodID j_mid_on_superuser_authorization_result;
  jmethodID j_mid_on_device_calcfg_result;
  jmethodID j_mid_on_device_calcfg_debug_string;
  jmethodID j_mid_on_device_calcfg_progress_report;
  jmethodID j_mid_on_channel_state;
  jmethodID j_mid_on_channel_basic_cfg;
  jmethodID j_mid_on_channel_function_set_result;
  jmethodID j_mid_on_channel_caption_set_result;
  jmethodID j_mid_on_channel_group_caption_set_result;
  jmethodID j_mid_on_location_caption_set_result;
  jmethodID j_mid_on_scene_caption_set_result;
  jmethodID j_mid_on_clients_reconnect_result;
  jmethodID j_mid_on_set_registration_enabled_result;
  jmethodID j_mid_on_zwave_reset_and_clear_result;
  jmethodID j_mid_on_zwave_add_node_result;
  jmethodID j_mid_on_zwave_remove_node_result;
  jmethodID j_mid_on_zwave_get_node_list_result;
  jmethodID j_mid_on_zwave_get_assigned_node_id_result;
  jmethodID j_mid_on_zwave_wake_up_settings_report;
  jmethodID j_mid_on_zwave_assign_node_id_result;
  jmethodID j_mid_on_zwave_set_wake_up_time_result;
  jmethodID j_mid_on_channel_config_update_or_result;
  jmethodID j_mid_on_device_config_update_or_result;
  jmethodID j_mid_on_action_execution_result;
} TAndroidSuplaClient;

jstring new_string_utf(JNIEnv *env, char *string);

void *supla_client_ptr(jlong _asc);

JNIEnv *supla_client_get_env(TAndroidSuplaClient *asc);

void supla_GetStringUtfChars(JNIEnv *env, jstring jstr, char *buff,
                             size_t size);

void supla_GetByteArrayElements(JNIEnv *env, jbyteArray barr, char *buff,
                                size_t size);

jobject supla_CallObjectMethod(JNIEnv *env, jclass cls, jobject obj,
                               const char *method_name, const char *type);

jlong supla_CallLongMethod(JNIEnv *env, jclass cls, jobject obj,
                           const char *method_name);

bool supla_CallLongObjectMethod(JNIEnv *env, jclass cls, jobject obj,
                                const char *method_name, jlong *result);

jint supla_CallIntMethod(JNIEnv *env, jclass cls, jobject obj,
                         const char *method_name);

bool supla_CallIntObjectMethod(JNIEnv *env, jclass cls, jobject obj,
                               const char *method_name, jint *result);

jshort supla_CallShortMethod(JNIEnv *env, jclass cls, jobject obj,
                             const char *method_name);

bool supla_CallShortObjectMethod(JNIEnv *env, jclass cls, jobject obj,
                                 const char *method_name, jshort *result);

jboolean supla_CallBooleanMethod(JNIEnv *env, jclass cls, jobject obj,
                                 const char *method_name);

jdouble supla_CallDoubleMethod(JNIEnv *env, jclass cls, jobject obj,
                               const char *method_name);

bool supla_CallDoubleObjectMethod(JNIEnv *env, jclass cls, jobject obj,
                                  const char *method_name, jdouble *result);

jobject supla_NewShort(JNIEnv *env, jshort value);

jobject supla_NewInt(JNIEnv *env, jint value);

jobject supla_NewDouble(JNIEnv *env, jdouble value);

jobject supla_NewArrayList(JNIEnv *env);

void supla_AddItemToArrayList(JNIEnv *env, jobject arr, jobject item);

jobject supla_NewEnum(JNIEnv *env, const char *cls_name, const char *enum_name);

jint supla_GetEnumValue(JNIEnv *env, jobject obj, const char *cls_name,
                        const char *method);

jint supla_GetEnumValue(JNIEnv *env, jobject obj, const char *cls_name);

jobject supla_NewEnumSet(JNIEnv *env, const char *cls_name);

void supla_AddItemToEnumSet(JNIEnv *env, jobject enum_set, jobject item);

int supla_GetListSize(JNIEnv *env, jobject list);

jobject supla_GetListItem(JNIEnv *env, jobject list, jint index);

#endif /*SUPLA_H_*/
