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

#ifndef ACTIONS_H_
#define ACTIONS_H_

#include <jni.h>

#include "proto.h"

#ifdef __cplusplus
extern "C" {
#endif

void getActionExecutionCallParams(JNIEnv *env, jobject action_params,
                                  int *action_id, void **param,
                                  unsigned _supla_int16_t *param_size,
                                  int *subject_type, int *subject_id);

JNIEXPORT jboolean JNICALL
Java_org_supla_android_lib_SuplaClient_scExecuteAction(JNIEnv *env,
                                                       jobject thiz, jlong _asc,
                                                       jobject action_params);

#ifdef __cplusplus
}
#endif
#endif /*ACTIONS_H_*/
