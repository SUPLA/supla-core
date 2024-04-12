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

#include "channel_config_roller_shutter.h"

#include <stdlib.h>

#include "supla.h"

jobject supla_cc_rs_to_jobject(JNIEnv *env, _supla_int_t channel_id,
                               _supla_int_t func, jlong crc32,
                               TChannelConfig_Rollershutter *config) {
  jclass config_cls = env->FindClass(
      "org/supla/android/data/source/remote/rollershutter/"
      "SuplaChannelRollerShutterConfig");

  jmethodID method_init =
      env->GetMethodID(config_cls, "<init>", "(ILjava/lang/Integer;JIIZZI)V");

  jint timeMargin = config->TimeMargin;

  jobject result = env->NewObject(
      config_cls, method_init, channel_id, supla_NewInt(env, func), crc32,
      (jint)config->ClosingTimeMS, (jint)config->OpeningTimeMS,
      config->MotorUpsideDown ? JNI_TRUE : JNI_FALSE,
      config->ButtonsUpsideDown ? JNI_TRUE : JNI_FALSE, timeMargin);

  env->DeleteLocalRef(config_cls);

  return result;
}
