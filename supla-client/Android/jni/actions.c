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

void get_action_execution_call_params(jobject jparams, int *action_id,
                                      TAction_RS_Parameters **rs_param,
                                      TAction_RGBW_Parameters **rgbw_param,
                                      int *subject_type, int *subject_id) {}

JNIEXPORT jboolean JNICALL
Java_org_supla_android_lib_SuplaClient_scExecuteAction(JNIEnv *env,
                                                       jobject thiz, jlong _asc,
                                                       jobject jparams) {
  jboolean result = JNI_FALSE;

  void *supla_client = supla_client_ptr(_asc);
  if (supla_client) {
    int action_id = 0;
    int subject_type = 0;
    int subject_id = 0;
    TAction_RS_Parameters *rs_param = NULL;
    TAction_RGBW_Parameters *rgbw_param = NULL;

    get_action_execution_call_params(jparams, &action_id, &rs_param,
                                     &rgbw_param, &subject_type, &subject_id);

    if (supla_client_execute_action(supla_client, action_id, rs_param,
                                    rgbw_param, subject_type, subject_id) > 0) {
      result = JNI_TRUE;
    }

    if (rs_param) {
      free(rs_param);
    }

    if (rgbw_param) {
      free(rgbw_param);
    }
  }

  return result;
}
