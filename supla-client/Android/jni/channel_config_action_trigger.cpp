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

#include "channel_config_action_trigger.h"

#include "supla.h"

jobject supla_cc_action_trigger_to_jobject(
    JNIEnv *env, _supla_int_t channel_id, _supla_int_t func, jlong crc32,
    TChannelConfig_ActionTrigger *action_trigger) {
  jclass config_cls = env->FindClass(
      "org/supla/android/data/source/remote/actiontrigger/"
      "SuplaChannelActionTriggerConfig");

  jmethodID method_init = env->GetMethodID(
      config_cls, "<init>", "(ILjava/lang/Integer;JS)V");

  jshort temporary_unlock_duration_sec =
      static_cast<jshort>(action_trigger->TemporaryUnlockDurationSec);

  jobject result = env->NewObject(
      config_cls, method_init, channel_id, supla_NewInt(env, func), crc32,
      temporary_unlock_duration_sec);

  env->DeleteLocalRef(config_cls);

  return result;
}
