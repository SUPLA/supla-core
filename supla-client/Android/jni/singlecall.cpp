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
#include "supla.h"

void throwResultException(JNIEnv *env, const char *message, int code) {
  jclass cls =
      env->FindClass("org/supla/android/lib/singlecall/ResultException");

  jmethodID init_mid =
      env->GetMethodID(cls, "<init>", "(Ljava/lang/String;I)V");
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

void get_auth_details(JNIEnv *env, jobject context, jobject auth_info,
                      TCS_ClientAuthorizationDetails *details) {
  jclass cls = env->FindClass("org/supla/android/profile/AuthInfo");

  getDecryptedByteArrayField(env, context, cls, auth_info, "getDecryptedGuid",
                             details->GUID, sizeof(details->GUID));

  getDecryptedByteArrayField(env, context, cls, auth_info,
                             "getDecryptedAuthKey", details->AuthKey,
                             sizeof(details->AuthKey));

  jboolean email_auth =
      supla_CallBooleanMethod(env, cls, auth_info, "getEmailAuth", "()Z");

  jstring server = nullptr;

  if (email_auth) {
    server = (jstring)supla_CallObjectMethod(
        env, cls, auth_info, "getServerForEmail", "()Ljava/lang/String;");

    jstring email = server = (jstring)supla_CallObjectMethod(
        env, cls, auth_info, "getEmailAddress", "()Ljava/lang/String;");

    supla_GetStringUtfChars(env, email, details->Email, sizeof(details->Email));
  } else {
    server = (jstring)supla_CallObjectMethod(
        env, cls, auth_info, "getServerForAccessID", "()Ljava/lang/String;");

    jstring pass = (jstring)supla_CallObjectMethod(
        env, cls, auth_info, "getAccessIDpwd", "()Ljava/lang/String;");

    supla_GetStringUtfChars(env, pass, details->AccessIDpwd,
                            sizeof(details->AccessIDpwd));

    details->AccessID =
        supla_CallIntMethod(env, cls, auth_info, "getAccessID", "()I");
  }

  supla_GetStringUtfChars(env, server, details->ServerName,
                          sizeof(details->ServerName));
}

void actionParamsToAction(JNIEnv *env, jobject action_params,
                          TCS_Action *action) {
  TAction_RS_Parameters *rs_param = nullptr;
  TAction_RGBW_Parameters *rgbw_param = nullptr;

  int subject_type = 0;

  getActionExecutionCallParams(env, action_params, &action->ActionId, &rs_param,
                               &rgbw_param, &subject_type, &action->SubjectId);

  action->ParamSize = 0;
  action->SubjectType = subject_type;

  if (rs_param) {
    action->ParamSize = sizeof(TAction_RS_Parameters);
    memcpy(action->Param, rs_param, action->ParamSize);
    free(rs_param);
  }

  if (rgbw_param) {
    action->ParamSize = sizeof(TAction_RGBW_Parameters);
    memcpy(action->Param, rgbw_param, action->ParamSize);
    free(rgbw_param);
  }
}

extern "C" JNIEXPORT void JNICALL
Java_org_supla_android_lib_singlecall_SingleCall_executeAction(
    JNIEnv *env, jobject thiz, jobject context, jobject auth_info,
    jobject action_params) {
  TCS_ClientAuthorizationDetails auth_details = {};
  get_auth_details(env, context, auth_info, &auth_details);
}
