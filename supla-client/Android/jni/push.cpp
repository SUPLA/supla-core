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

JNIEXPORT jboolean JNICALL
Java_org_supla_android_lib_SuplaClient_scRegisterPushNotificationClientToken(
    JNIEnv *env, jobject thiz, jlong _asc, jint app_id, jstring token) {
  jboolean result = JNI_FALSE;

  void *supla_client = supla_client_ptr(_asc);

  if (supla_client) {
    char *tkn = token ? (char *)env->GetStringUTFChars(token, 0) : nullptr;

    TCS_RegisterPnClientToken reg = {};
    reg.Platform = PLATFORM_ANDROID;
    reg.AppId = app_id;

    if (tkn) {
      reg.TokenSize = strnlen(tkn, SUPLA_PN_CLIENT_TOKEN_MAXSIZE) + 1;
      reg.RealTokenSize = strnlen(tkn, 4096) + 1;

      if (reg.TokenSize > SUPLA_PN_CLIENT_TOKEN_MAXSIZE) {
        reg.TokenSize = SUPLA_PN_CLIENT_TOKEN_MAXSIZE;
      }

      snprintf((char *)reg.Token, reg.TokenSize, "%s", tkn);
    }

    result = supla_client_pn_register_client_token(supla_client, &reg) == 1
                 ? JNI_TRUE
                 : JNI_FALSE;

    if (tkn) {
      env->ReleaseStringUTFChars(token, tkn);
    }
  }

  return result;
}
