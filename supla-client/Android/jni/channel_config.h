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

#ifndef CHANNEL_CONFIG_H_
#define CHANNEL_CONFIG_H_

#include <jni.h>

#include "proto.h"
#include "supla-client.h"
#include "supla.h"

#ifdef __cplusplus
extern "C" {
#endif

jobject supla_config_result_to_jobject(JNIEnv *env, int result);

void supla_channel_config_init(JNIEnv *env, jclass oclass,
                               TAndroidSuplaClient *asc,
                               TSuplaClientCfg *sclient_cfg);

JNIEXPORT jboolean JNICALL
Java_org_supla_android_lib_SuplaClient_scGetChannelConfig(
    JNIEnv *env, jobject thiz, jlong _asc, jint channel_id, jobject type);

JNIEXPORT jboolean JNICALL
Java_org_supla_android_lib_SuplaClient_scSetChannelConfig(JNIEnv *env,
                                                          jobject thiz,
                                                          jlong _asc,
                                                          jobject config);

#ifdef __cplusplus
}
#endif
#endif /*CHANNEL_CONFIG_H_*/
