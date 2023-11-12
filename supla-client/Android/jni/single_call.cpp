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

#include <android/log.h>
#include <stdlib.h>
#include <string.h>

#include "actions.h"
#include "proto.h"
#include "push.h"
#include "supla.h"
#include "suplasinglecall.h"

void throwException(JNIEnv *env, const char *class_path, int result) {
  jclass cls = env->FindClass(class_path);

  jmethodID init_mid = env->GetMethodID(cls, "<init>", "(I)V");

  jthrowable exception = (jthrowable)env->NewObject(cls, init_mid, result);
  env->Throw(exception);
}

void throwResultException(JNIEnv *env, int result) {
  throwException(env, "org/supla/android/lib/singlecall/ResultException",
                 result);
}

void getDecryptedByteArrayField(JNIEnv *env, jobject context, jclass cls,
                                jobject auth_info, const char *method_name,
                                char *buff, int size) {
  jmethodID method_id =
      env->GetMethodID(cls, method_name, "(Landroid/content/Context;)[B");

  jbyteArray barr =
      (jbyteArray)env->CallObjectMethod(auth_info, method_id, context);

  supla_GetByteArrayElements(env, barr, buff, size);
}

void getAuthDetails(JNIEnv *env, jobject context, jobject auth_info,
                    TCS_ClientAuthorizationDetails *details,
                    int *protocol_version) {
  jclass cls = env->FindClass("org/supla/android/profile/AuthInfo");

  getDecryptedByteArrayField(env, context, cls, auth_info, "getDecryptedGuid",
                             details->GUID, sizeof(details->GUID));

  getDecryptedByteArrayField(env, context, cls, auth_info,
                             "getDecryptedAuthKey", details->AuthKey,
                             sizeof(details->AuthKey));

  jboolean email_auth =
      supla_CallBooleanMethod(env, cls, auth_info, "getEmailAuth");

  jstring server = nullptr;

  if (email_auth) {
    server = (jstring)supla_CallObjectMethod(
        env, cls, auth_info, "getServerForEmail", "()Ljava/lang/String;");

    jstring email = (jstring)supla_CallObjectMethod(
        env, cls, auth_info, "getEmailAddress", "()Ljava/lang/String;");

    supla_GetStringUtfChars(env, email, details->Email, sizeof(details->Email));
  } else {
    server = (jstring)supla_CallObjectMethod(
        env, cls, auth_info, "getServerForAccessID", "()Ljava/lang/String;");

    jstring pass = (jstring)supla_CallObjectMethod(
        env, cls, auth_info, "getAccessIDpwd", "()Ljava/lang/String;");

    supla_GetStringUtfChars(env, pass, details->AccessIDpwd,
                            sizeof(details->AccessIDpwd));

    details->AccessID = supla_CallIntMethod(env, cls, auth_info, "getAccessID");
  }

  supla_GetStringUtfChars(env, server, details->ServerName,
                          sizeof(details->ServerName));

  *protocol_version =
      supla_CallIntMethod(env, cls, auth_info, "getPreferredProtocolVersion");
}

void actionParamsToAction(JNIEnv *env, jobject action_params,
                          TCS_Action *action) {
  void *param = nullptr;
  unsigned _supla_int16_t param_size = 0;
  int subject_type = 0;

  getActionExecutionCallParams(env, action_params, &action->ActionId, &param,
                               &param_size, &subject_type, &action->SubjectId);

  action->SubjectType = subject_type;

  if (param) {
    if (param_size <= SUPLA_ACTION_PARAM_MAXSIZE) {
      action->ParamSize = param_size;
      memcpy(action->Param, param, param_size);
    }
    free(param);
  }
}

jobject getChannelValueObject(JNIEnv *env) {
  jclass cls = env->FindClass("org/supla/android/lib/singlecall/ChannelValue");
  jmethodID methodID = env->GetMethodID(cls, "<init>", "()V");

  return env->NewObject(cls, methodID);
}

jobject getStateOfOpeningObject(JNIEnv *env, jboolean is_open) {
  jclass cls =
      env->FindClass("org/supla/android/lib/singlecall/StateOfOpening");
  jmethodID methodID = env->GetMethodID(cls, "<init>", "(Z)V");

  return env->NewObject(cls, methodID, is_open);
}

jobject getTemperatureAndHumidityObject(JNIEnv *env, int function,
                                        char value[SUPLA_CHANNELVALUE_SIZE]) {
  jclass cls =
      env->FindClass("org/supla/android/lib/singlecall/TemperatureAndHumidity");
  jmethodID methodID = env->GetMethodID(
      cls, "<init>", "(Ljava/lang/Double;Ljava/lang/Double;)V");

  double dtemp = -273;
  double dhum = -1;

  switch (function) {
    case SUPLA_CHANNELFNC_THERMOMETER:
      memcpy(&dtemp, value, sizeof(double));
      break;
    case SUPLA_CHANNELFNC_HUMIDITY:
    case SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE: {
      int n = 0;
      memcpy(&n, value, 4);
      dtemp = n / 1000.00;

      memcpy(&n, &value[4], 4);
      dhum = n / 1000.00;
    } break;
  }

  jobject temp = nullptr;
  jobject hum = nullptr;

  if (dtemp > -273) {
    temp = supla_NewDouble(env, dtemp);
  }

  if (dhum >= 0 && dhum <= 100) {
    hum = supla_NewDouble(env, dhum);
  }

  return env->NewObject(cls, methodID, temp, hum);
}

jobject getRollerShutterPositionObject(JNIEnv *env, jint position) {
  jclass cls =
      env->FindClass("org/supla/android/lib/singlecall/RollerShutterPosition");
  jmethodID methodID =
      env->GetMethodID(cls, "<init>", "(Ljava/lang/Integer;Z)V");

  if (position >= 0) {
    return env->NewObject(cls, methodID, supla_NewInt(env, position), false);
  }

  return env->NewObject(cls, methodID, nullptr, true);
}

extern "C" JNIEXPORT void JNICALL
Java_org_supla_android_lib_singlecall_SingleCall_executeAction(
    JNIEnv *env, jobject thiz, jobject context, jobject auth_info,
    jobject action_params) {
  TCS_ClientAuthorizationDetails auth_details = {};
  int protocol_version = 0;
  getAuthDetails(env, context, auth_info, &auth_details, &protocol_version);

  TCS_Action action = {};
  actionParamsToAction(env, action_params, &action);

  supla_single_call single_call(&auth_details, protocol_version);
  int result = single_call.execute_action(&action);
  if (result != SUPLA_RESULTCODE_TRUE) {
    throwResultException(env, result);
  }
}

extern "C" JNIEXPORT void JNICALL
Java_org_supla_android_lib_singlecall_SingleCall_registerPushNotificationClientToken(
    JNIEnv *env, jobject thiz, jobject context, jobject auth_info, jint app_id,
    jstring token) {
  TCS_ClientAuthorizationDetails auth_details = {};
  int protocol_version = 0;
  getAuthDetails(env, context, auth_info, &auth_details, &protocol_version);

  TCS_PnClientToken pn_token = {};

  set_token_details(env, &pn_token, app_id, token);

  supla_single_call single_call(&auth_details, protocol_version);
  int result = single_call.register_pn_client_token(&pn_token);
  if (result != SUPLA_RESULTCODE_TRUE) {
    throwResultException(env, result);
  }
}

extern "C" JNIEXPORT jobject JNICALL
Java_org_supla_android_lib_singlecall_SingleCall_getChannelValue(
    JNIEnv *env, jobject thiz, jobject context, jobject auth_info,
    jint channel_id) {
  TCS_ClientAuthorizationDetails auth_details = {};
  int protocol_version = 0;
  getAuthDetails(env, context, auth_info, &auth_details, &protocol_version);

  supla_single_call single_call(&auth_details, protocol_version);

  TSC_GetChannelValueResult vresult = {};
  int r = single_call.get_channel_value(channel_id, &vresult);

  if (r != SUPLA_RESULTCODE_TRUE) {
    throwResultException(env, r);
    return nullptr;
  }

  switch (vresult.Function) {
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGATEWAYLOCK:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGATE:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEDOORLOCK:
      return getStateOfOpeningObject(env, vresult.Value.sub_value[0] == 0);
      break;
    case SUPLA_CHANNELFNC_OPENINGSENSOR_GATEWAY:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_GATE:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_GARAGEDOOR:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_DOOR:
    case SUPLA_CHANNELFNC_NOLIQUIDSENSOR:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_ROLLERSHUTTER:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_ROOFWINDOW:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_WINDOW:
    case SUPLA_CHANNELFNC_MAILSENSOR:
      return getStateOfOpeningObject(env, vresult.Value.value[0] == 0);
    case SUPLA_CHANNELFNC_THERMOMETER:
    case SUPLA_CHANNELFNC_HUMIDITY:
    case SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE:
      return getTemperatureAndHumidityObject(env, vresult.Function,
                                             vresult.Value.value);
      break;
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROOFWINDOW:
      return getRollerShutterPositionObject(env, vresult.Value.value[0]);
  }

  return getChannelValueObject(env);
}
