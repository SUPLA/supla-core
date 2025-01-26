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

#include "actions.h"

#include <android/log.h>
#include <stdlib.h>

#include "supla-client.h"
#include "supla.h"

jobject supla_action_id_to_jobject(JNIEnv *env, int action_id) {
  jclass result_cls = env->FindClass("org/supla/android/lib/actions/ActionId");

  jmethodID result_init = env->GetStaticMethodID(
      result_cls, "valueOf",
      "(Ljava/lang/String;)Lorg/supla/android/lib/actions/ActionId;");

  char enum_name[30] = {};

  switch (action_id) {
    case ACTION_OPEN:
      snprintf(enum_name, sizeof(enum_name), "OPEN");
      break;
    case ACTION_CLOSE:
      snprintf(enum_name, sizeof(enum_name), "CLOSE");
      break;
    case ACTION_SHUT:
      snprintf(enum_name, sizeof(enum_name), "SHUT");
      break;
    case ACTION_REVEAL:
      snprintf(enum_name, sizeof(enum_name), "REVEAL");
      break;
    case ACTION_REVEAL_PARTIALLY:
      snprintf(enum_name, sizeof(enum_name), "REVEAL_PARTIALLY");
      break;
    case ACTION_SHUT_PARTIALLY:
      snprintf(enum_name, sizeof(enum_name), "SHUT_PARTIALLY");
      break;
    case ACTION_TURN_ON:
      snprintf(enum_name, sizeof(enum_name), "TURN_ON");
      break;
    case ACTION_TURN_OFF:
      snprintf(enum_name, sizeof(enum_name), "TURN_OFF");
      break;
    case ACTION_SET_RGBW_PARAMETERS:
      snprintf(enum_name, sizeof(enum_name), "SET_RGBW_PARAMETERS");
      break;
    case ACTION_OPEN_CLOSE:
      snprintf(enum_name, sizeof(enum_name), "OPEN_CLOSE");
      break;
    case ACTION_STOP:
      snprintf(enum_name, sizeof(enum_name), "STOP");
      break;
    case ACTION_TOGGLE:
      snprintf(enum_name, sizeof(enum_name), "TOGGLE");
      break;
    case ACTION_UP_OR_STOP:
      snprintf(enum_name, sizeof(enum_name), "UP_OR_STOP");
      break;
    case ACTION_DOWN_OR_STOP:
      snprintf(enum_name, sizeof(enum_name), "DOWN_OR_STOP");
      break;
    case ACTION_STEP_BY_STEP:
      snprintf(enum_name, sizeof(enum_name), "STEP_BY_STEP");
      break;
    case ACTION_UP:
      snprintf(enum_name, sizeof(enum_name), "UP");
      break;
    case ACTION_DOWN:
      snprintf(enum_name, sizeof(enum_name), "DOWN");
      break;
    case ACTION_HVAC_SET_PARAMETERS:
      snprintf(enum_name, sizeof(enum_name), "SET_HVAC_PARAMETERS");
      break;
    case ACTION_EXECUTE:
      snprintf(enum_name, sizeof(enum_name), "EXECUTE");
      break;
    case ACTION_INTERRUPT:
      snprintf(enum_name, sizeof(enum_name), "INTERRUPT");
      break;
    case ACTION_INTERRUPT_AND_EXECUTE:
      snprintf(enum_name, sizeof(enum_name), "INTERRUPT_AND_EXECUTE");
      break;
  }

  return enum_name[0] == 0
             ? env->NewGlobalRef(nullptr)
             : env->CallStaticObjectMethod(result_cls, result_init,
                                           env->NewStringUTF(enum_name));
}

jobject supla_subject_type_to_jobject(JNIEnv *env, int subject_type) {
  jclass result_cls =
      env->FindClass("org/supla/android/lib/actions/SubjectType");

  jmethodID result_init = env->GetStaticMethodID(
      result_cls, "valueOf",
      "(Ljava/lang/String;)Lorg/supla/android/lib/actions/SubjectType;");

  char enum_name[30] = {};

  switch (subject_type) {
    case ACTION_SUBJECT_TYPE_CHANNEL:
      snprintf(enum_name, sizeof(enum_name), "CHANNEL");
      break;
    case ACTION_SUBJECT_TYPE_CHANNEL_GROUP:
      snprintf(enum_name, sizeof(enum_name), "GROUP");
      break;
    case ACTION_SUBJECT_TYPE_SCENE:
      snprintf(enum_name, sizeof(enum_name), "SCENE");
      break;
  }

  return enum_name[0] == 0
             ? env->NewGlobalRef(nullptr)
             : env->CallStaticObjectMethod(result_cls, result_init,
                                           env->NewStringUTF(enum_name));
}

void supla_cb_on_action_execution_result(void *_suplaclient, void *user_data,
                                         TSC_ActionExecutionResult *result) {
  ASC_VAR_DECLARATION();
  ENV_VAR_DECLARATION();

  if (asc->j_mid_on_action_execution_result == nullptr || result == nullptr) {
    return;
  }

  jobject jaction_id = supla_action_id_to_jobject(env, result->ActionId);
  jobject jsubject_type =
      supla_subject_type_to_jobject(env, result->SubjectType);

  env->CallVoidMethod(asc->j_obj, asc->j_mid_on_action_execution_result,
                      jaction_id, jsubject_type, (jint)result->SubjectId);

  env->DeleteLocalRef(jaction_id);
  env->DeleteLocalRef(jsubject_type);
}

void supla_actions_init(JNIEnv *env, jclass oclass, TAndroidSuplaClient *asc,
                        TSuplaClientCfg *sclient_cfg) {
  asc->j_mid_on_action_execution_result =
      env->GetMethodID(oclass, "onActionExecutionResult",
                       "(Lorg/supla/android/lib/actions/ActionId;Lorg/supla/"
                       "android/lib/actions/SubjectType;I)V");

  sclient_cfg->cb_on_action_execution_result =
      supla_cb_on_action_execution_result;
}

void getActionExecutionCallParams(JNIEnv *env, jobject action_params,
                                  int *action_id, void **param,
                                  unsigned _supla_int16_t *param_size,
                                  int *subject_type, int *subject_id) {
  jclass cls = env->FindClass("org/supla/android/lib/actions/ActionParameters");

  jclass ss_cls = env->FindClass(
      "org/supla/android/lib/actions/ShadingSystemActionParameters");

  jclass rgbw_cls =
      env->FindClass("org/supla/android/lib/actions/RgbwActionParameters");

  jclass hvac_cls =
      env->FindClass("org/supla/android/lib/actions/HvacActionParameters");

  if (env->IsInstanceOf(action_params, ss_cls)) {
    cls = ss_cls;
    TAction_ShadingSystem_Parameters *ss_param =
        (TAction_ShadingSystem_Parameters *)malloc(
            sizeof(TAction_ShadingSystem_Parameters));
    *ss_param = {};

    ss_param->Percentage =
        supla_CallShortMethod(env, cls, action_params, "getPercentage");

    ss_param->Tilt = supla_CallShortMethod(env, cls, action_params, "getTilt");

    jboolean percentage_as_delta = supla_CallBooleanMethod(
        env, cls, action_params, "getPercentageAsDelta");

    jboolean tilt_as_delta =
        supla_CallBooleanMethod(env, cls, action_params, "getTiltAsDelta");

    if (percentage_as_delta) {
      ss_param->Flags |= SSP_FLAG_PERCENTAGE_AS_DELTA;
    }

    if (tilt_as_delta) {
      ss_param->Flags |= SSP_FLAG_TILT_AS_DELTA;
    }

    *param = ss_param;
    *param_size = sizeof(TAction_ShadingSystem_Parameters);

  } else if (env->IsInstanceOf(action_params, rgbw_cls)) {
    cls = rgbw_cls;
    TAction_RGBW_Parameters *rgbw_param =
        (TAction_RGBW_Parameters *)malloc(sizeof(TAction_RGBW_Parameters));
    *rgbw_param = {};

    rgbw_param->Brightness =
        supla_CallShortMethod(env, cls, action_params, "getBrightness");
    rgbw_param->ColorBrightness =
        supla_CallShortMethod(env, cls, action_params, "getColorBrightness");
    rgbw_param->Color =
        (jlong)supla_CallLongMethod(env, cls, action_params, "getColor");
    rgbw_param->ColorRandom =
        supla_CallBooleanMethod(env, cls, action_params, "getColorRandom") ? 1
                                                                           : 0;
    rgbw_param->OnOff =
        supla_CallBooleanMethod(env, cls, action_params, "getOnOff") ? 1 : 0;

    *param = rgbw_param;
    *param_size = sizeof(TAction_RGBW_Parameters);

  } else if (env->IsInstanceOf(action_params, hvac_cls)) {
    cls = hvac_cls;
    TAction_HVAC_Parameters *hvac_param =
        (TAction_HVAC_Parameters *)malloc(sizeof(TAction_HVAC_Parameters));
    *hvac_param = {};

    jlong l = 0;
    if (supla_CallLongObjectMethod(env, cls, action_params, "getDurationSec",
                                   &l)) {
      hvac_param->DurationSec = l;
    }

    jobject mode_enum = supla_CallObjectMethod(
        env, cls, action_params, "getMode",
        "()Lorg/supla/android/data/source/remote/hvac/SuplaHvacMode;");

    if (!env->IsSameObject(mode_enum, nullptr)) {
      hvac_param->Mode = supla_GetEnumValue(
          env, mode_enum,
          "org/supla/android/data/source/remote/hvac/SuplaHvacMode");
    }
    jshort s = 0;

    if (supla_CallShortObjectMethod(env, cls, action_params,
                                    "getSetpointTemperatureHeat", &s)) {
      hvac_param->SetpointTemperatureHeat = s;
      hvac_param->Flags |= SUPLA_HVAC_VALUE_FLAG_SETPOINT_TEMP_HEAT_SET;
    }

    if (supla_CallShortObjectMethod(env, cls, action_params,
                                    "getSetpointTemperatureCool", &s)) {
      hvac_param->SetpointTemperatureCool = s;
      hvac_param->Flags |= SUPLA_HVAC_VALUE_FLAG_SETPOINT_TEMP_COOL_SET;
    }

    *param = hvac_param;
    *param_size = sizeof(TAction_HVAC_Parameters);
  }

  jclass action_id_cls =
      env->FindClass("org/supla/android/lib/actions/ActionId");

  jobject action_id_obj =
      supla_CallObjectMethod(env, cls, action_params, "getAction",
                             "()Lorg/supla/android/lib/actions/ActionId;");

  *action_id =
      supla_CallIntMethod(env, action_id_cls, action_id_obj, "getValue");

  jobject subject_type_obj =
      supla_CallObjectMethod(env, cls, action_params, "getSubjectType",
                             "()Lorg/supla/android/lib/actions/SubjectType;");

  jclass subject_type_cls =
      env->FindClass("org/supla/android/lib/actions/SubjectType");

  *subject_type =
      supla_CallIntMethod(env, subject_type_cls, subject_type_obj, "getValue");

  *subject_id = supla_CallIntMethod(env, cls, action_params, "getSubjectId");

  __android_log_print(ANDROID_LOG_DEBUG, log_tag, "Action ID: %i,%i,%i",
                      *action_id, *subject_type, *subject_id);
}

JNIEXPORT jboolean JNICALL
Java_org_supla_android_lib_SuplaClient_scExecuteAction(JNIEnv *env,
                                                       jobject thiz, jlong _asc,
                                                       jobject action_params) {
  jboolean result = JNI_FALSE;

  void *supla_client = supla_client_ptr(_asc);
  if (supla_client) {
    int action_id = 0;
    int subject_type = 0;
    int subject_id = 0;
    void *param = nullptr;
    unsigned _supla_int16_t param_size = 0;

    getActionExecutionCallParams(env, action_params, &action_id, &param,
                                 &param_size, &subject_type, &subject_id);

    if (supla_client_execute_action(supla_client, action_id, param, param_size,
                                    subject_type, subject_id) > 0) {
      result = JNI_TRUE;
    }

    if (param) {
      free(param);
    }
  }

  return result;
}
