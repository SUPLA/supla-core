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

#include "channel_config.h"

#include <android/log.h>
#include <stdlib.h>

#include "channel_config_hvac.h"
#include "channel_config_weekly_schedule.h"

jobject supla_config_result_to_jobject(JNIEnv *env, int result) {
  jclass result_cls = env->FindClass(
      "org/supla/"
      "android/data/source/remote/ChannelConfigResult");

  jmethodID result_init =
      env->GetStaticMethodID(result_cls, "valueOf",
                             "(Ljava/lang/String;)Lorg/supla/android/data/"
                             "source/remote/ChannelConfigResult;");

  char enum_name[30] = {};

  switch (result) {
    case SUPLA_CONFIG_RESULT_FALSE:
      snprintf(enum_name, sizeof(enum_name), "RESULT_FALSE");
      break;
    case SUPLA_CONFIG_RESULT_TRUE:
      snprintf(enum_name, sizeof(enum_name), "RESULT_TRUE");
      break;
    case SUPLA_CONFIG_RESULT_DATA_ERROR:
      snprintf(enum_name, sizeof(enum_name), "DATA_ERROR");
      break;
    case SUPLA_CONFIG_RESULT_TYPE_NOT_SUPPORTED:
      snprintf(enum_name, sizeof(enum_name), "TYPE_NOT_SUPPORTED");
      break;
    case SUPLA_CONFIG_RESULT_FUNCTION_NOT_SUPPORTED:
      snprintf(enum_name, sizeof(enum_name), "FUNCTION_NOT_SUPPORTED");
      break;
    case SUPLA_CONFIG_RESULT_LOCAL_CONFIG_DISABLED:
      snprintf(enum_name, sizeof(enum_name), "LOCAL_CONFIG_DISABLED");
      break;
    case SUPLA_CONFIG_RESULT_NOT_ALLOWED:
      snprintf(enum_name, sizeof(enum_name), "NOT_ALLOWED");
      break;
  }

  return enum_name[0] == 0
             ? env->NewGlobalRef(nullptr)
             : env->CallStaticObjectMethod(result_cls, result_init,
                                           env->NewStringUTF(enum_name));
}

jobject supla_channel_config_to_jobject(JNIEnv *env,
                                        TSCS_ChannelConfig *config) {
  if (config && config->ConfigSize) {
    switch (config->Func) {
      case SUPLA_CHANNELFNC_HVAC_THERMOSTAT_HEAT:
      case SUPLA_CHANNELFNC_HVAC_THERMOSTAT_COOL:
      case SUPLA_CHANNELFNC_HVAC_THERMOSTAT_AUTO:
        if (config->ConfigType == SUPLA_CONFIG_TYPE_DEFAULT &&
            sizeof(TChannelConfig_HVAC) == config->ConfigSize) {
          return supla_cc_hvac_to_jobject(
              env, config->ChannelId, config->Func,
              (TChannelConfig_HVAC *)config->Config);
        } else if (config->ConfigType == SUPLA_CONFIG_TYPE_WEEKLY_SCHEDULE &&
                   sizeof(TChannelConfig_WeeklySchedule) ==
                       config->ConfigSize) {
          return supla_cc_weekly_schedule_to_jobject(
              env, config->ChannelId, config->Func,
              (TChannelConfig_WeeklySchedule *)config->Config);
        }
        break;
    }
  }

  jclass config_cls = env->FindClass(
      "org/supla/"
      "android/data/source/remote/SuplaChannelConfig");

  jmethodID init_method =
      env->GetMethodID(config_cls, "<init>", "(ILjava/lang/Integer;)V");
  return env->NewObject(config_cls, init_method, config->ChannelId,
                        supla_NewInt(env, config->Func));
}

void supla_cb_on_channel_config_update_or_result(
    void *_suplaclient, void *user_data,
    TSC_ChannelConfigUpdateOrResult *config) {
  ASC_VAR_DECLARATION();
  ENV_VAR_DECLARATION();

  jobject jresult = supla_config_result_to_jobject(env, config->Result);
  jobject jconfig = config->Result == SUPLA_CONFIG_RESULT_TRUE
                        ? supla_channel_config_to_jobject(env, &config->Config)
                        : env->NewGlobalRef(nullptr);

  env->CallVoidMethod(asc->j_obj, asc->j_mid_on_channel_config_update_or_result,
                      jconfig, jresult);
}

void supla_channel_config_init(JNIEnv *env, jclass oclass,
                               TAndroidSuplaClient *asc,
                               TSuplaClientCfg *sclient_cfg) {
  asc->j_mid_on_channel_config_update_or_result = env->GetMethodID(
      oclass, "onChannelConfigUpdateOrResult",
      "(Lorg/supla/android/data/source/remote/SuplaChannelConfig;Lorg/supla/"
      "android/data/source/remote/ChannelConfigResult;)V");

  sclient_cfg->cb_on_channel_config_update_or_result =
      supla_cb_on_channel_config_update_or_result;
}

JNIEXPORT jboolean JNICALL
Java_org_supla_android_lib_SuplaClient_scGetChannelConfig(
    JNIEnv *env, jobject thiz, jlong _asc, jint channel_id, jobject type) {
  jboolean result = JNI_FALSE;

  void *supla_client = supla_client_ptr(_asc);
  if (supla_client) {
    TCS_GetChannelConfigRequest request = {};
    request.ChannelId = channel_id;

    jclass cls = env->FindClass(
        "org/supla/android/data/source/remote/ChannelConfigType");
    request.ConfigType = supla_CallIntMethod(env, cls, type, "getValue");

    result = supla_client_get_channel_config(supla_client, &request) > 0
                 ? JNI_TRUE
                 : JNI_FALSE;
  }

  return result;
}

JNIEXPORT jboolean JNICALL
Java_org_supla_android_lib_SuplaClient_scSetChannelConfig(JNIEnv *env,
                                                          jobject thiz,
                                                          jlong _asc,
                                                          jobject config) {
  jboolean result = JNI_FALSE;

  void *supla_client = supla_client_ptr(_asc);
  if (supla_client) {
    TSCS_ChannelConfig scs_config = {};
    if (!supla_cc_jobject_to_weekly_schedule(env, config, &scs_config)) {
      return JNI_FALSE;
    }

    jclass cls = env->FindClass(
        "org/supla/android/data/source/remote/SuplaChannelConfig");

    scs_config.ChannelId = supla_CallIntMethod(env, cls, config, "getRemoteId");

    result = supla_client_set_channel_config(supla_client, &scs_config) > 0
                 ? JNI_TRUE
                 : JNI_FALSE;
  }

  return result;
}
