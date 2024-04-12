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

#include "channel_config_facade_blind.h"

#include <stdlib.h>

#include "supla.h"

jobject supla_cc_fb_type_to_object(JNIEnv *env, unsigned char type) {
  char enum_name[40] = {};

  switch (type) {
    case SUPLA_FACADEBLIND_TYPE_STANDS_IN_POSITION_WHILE_TILTING:
      snprintf(enum_name, sizeof(enum_name),
               "STANDS_IN_POSITION_WHILE_TILTING");
      break;
    case SUPLA_FACADEBLIND_TYPE_CHANGES_POSITION_WHILE_TILTING:
      snprintf(enum_name, sizeof(enum_name), "CHANGES_POSITION_WHILE_TILTING");
      break;
    case SUPLA_FACADEBLIND_TYPE_TILTS_ONLY_WHEN_FULLY_CLOSED:
      snprintf(enum_name, sizeof(enum_name), "TILTS_ONLY_WHEN_FULLY_CLOSED");
      break;
    default:
      snprintf(enum_name, sizeof(enum_name), "UNKNOWN");
      break;
  }

  return supla_NewEnum(
      env,
      "org/supla/android/data/source/remote/rollershutter/SuplaFacadeBlindType",
      enum_name);
}

jobject supla_cc_fb_to_jobject(JNIEnv *env, _supla_int_t channel_id,
                               _supla_int_t func, jlong crc32,
                               TChannelConfig_FacadeBlind *config) {
  jclass config_cls = env->FindClass(
      "org/supla/android/data/source/remote/rollershutter/"
      "SuplaChannelFacadeBlindConfig");

  jmethodID method_init = env->GetMethodID(
      config_cls, "<init>",
      "(ILjava/lang/Integer;JIIIZZIII;ILorg/supla/android/data/source/remote/"
      "rollershutter/SuplaFacadeBlindType;)V");

  jint timeMargin = config->TimeMargin;
  jint tilt0Angle = config->Tilt0Angle;
  jint tilt100Angle = config->Tilt100Angle;

  jobject fb_type = supla_cc_fb_type_to_object(env, config->FacadeBlindType);

  jobject result = env->NewObject(
      config_cls, method_init, channel_id, supla_NewInt(env, func), crc32,
      (jint)config->ClosingTimeMS, (jint)config->OpeningTimeMS,
      (jint)config->TiltingTimeMS,
      config->MotorUpsideDown ? JNI_TRUE : JNI_FALSE,
      config->ButtonsUpsideDown ? JNI_TRUE : JNI_FALSE, timeMargin, tilt0Angle,
      tilt100Angle, fb_type);

  env->DeleteLocalRef(config_cls);

  return result;
}
