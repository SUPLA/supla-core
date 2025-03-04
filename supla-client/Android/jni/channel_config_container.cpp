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

#include "channel_config_container.h"

#include <stdlib.h>

#include "supla.h"

jobject supla_cc_sensor_info_to_object(JNIEnv *env, _supla_int_t channel_id, char fillLevel) {
  jclass config_cls = env->FindClass(
      "org/supla/android/data/source/remote/container/SuplaSensorInfo");
      
  jmethodID method_init = env->GetMethodID(config_cls, "<init>", "(II)V");
  
  jobject result = env->NewObject(config_cls, method_init, (jint) fillLevel, channel_id);
  
  env->DeleteLocalRef(config_cls);
  
  return result;
}

jobject supla_cc_container_sensors_to_jobject(
    JNIEnv *env, int sensorsCount, TContainer_SensorInfo *sensors) {
  jobject jarr = supla_NewArrayList(env);

  for (int i=0; i<sensorsCount; i++) {
    TContainer_SensorInfo sensor = sensors[i];
    if (sensor.FillLevel != 0 && sensor.ChannelId != 0) {
      jobject sensorObject = supla_cc_sensor_info_to_object(
          env, sensor.ChannelId, sensor.FillLevel);
          supla_AddItemToArrayList(env, jarr, sensorObject);
    }
  }

  return jarr;
}

jobject supla_cc_container_to_jobject(JNIEnv *env, _supla_int_t channel_id,
                                      _supla_int_t func, jlong crc32,
                                      TChannelConfig_Container *container) {
  jclass config_cls = env->FindClass(
      "org/supla/android/data/source/remote/container/SuplaChannelContainerConfig");

  jmethodID method_init = env->GetMethodID(
      config_cls, "<init>",
      "(ILjava/lang/Integer;JIIIIZLjava/util/List;)V");
      
  int sensorsCount = sizeof(container->SensorInfo) / sizeof(container->SensorInfo[0]);
  jobject sensors = supla_cc_container_sensors_to_jobject(
      env, sensorsCount, container->SensorInfo);

  jobject result = env->NewObject(
      config_cls, 
      method_init, 
      channel_id, 
      supla_NewInt(env, func), 
      crc32,
      (jint)container->WarningAboveLevel, 
      (jint)container->AlarmAboveLevel, 
      (jint)container->WarningBelowLevel, 
      (jint)container->AlarmBelowLevel, 
      container->MuteAlarmSoundWithoutAdditionalAuth ? JNI_TRUE : JNI_FALSE, 
      sensors);

  env->DeleteLocalRef(config_cls);

  return result;
}
