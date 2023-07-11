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

#include "push.h"

#include <android/log.h>
#include <stdio.h>
#include <string.h>

#include "supla-client.h"
#include "supla.h"

void set_token_details(JNIEnv *env, TCS_PnClientToken *pn_token, jint app_id,
                       jstring token) {
  pn_token->Platform = PLATFORM_ANDROID;
  pn_token->AppId = app_id;
  char *tkn = token ? (char *)env->GetStringUTFChars(token, 0) : nullptr;

  if (tkn) {
    pn_token->TokenSize = strnlen(tkn, SUPLA_PN_CLIENT_TOKEN_MAXSIZE) + 1;
    pn_token->RealTokenSize = strnlen(tkn, 4096) + 1;

    if (pn_token->TokenSize > SUPLA_PN_CLIENT_TOKEN_MAXSIZE) {
      pn_token->TokenSize = SUPLA_PN_CLIENT_TOKEN_MAXSIZE;
    }

    snprintf((char *)pn_token->Token, pn_token->TokenSize, "%s", tkn);
    env->ReleaseStringUTFChars(token, tkn);
  }
}

JNIEXPORT jboolean JNICALL
Java_org_supla_android_lib_SuplaClient_scRegisterPushNotificationClientToken(
    JNIEnv *env, jobject thiz, jlong _asc, jint app_id, jstring token) {
  jboolean result = JNI_FALSE;

  void *supla_client = supla_client_ptr(_asc);

  if (supla_client) {
    TCS_RegisterPnClientToken reg = {};

    set_token_details(env, &reg.Token, app_id, token);

    result = supla_client_pn_register_client_token(supla_client, &reg) == 1
                 ? JNI_TRUE
                 : JNI_FALSE;
  }

  return result;
}
