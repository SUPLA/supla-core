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

#include "channel_config_hvac.h"

#include <stdlib.h>

#include "supla.h"

jobject supla_cc_hvac_thermometer_type_to_jobject(JNIEnv *env,
                                                  unsigned char type) {
  char enum_name[20] = {};

  switch (type) {
    case SUPLA_HVAC_AUX_THERMOMETER_TYPE_DISABLED:
      snprintf(enum_name, sizeof(enum_name), "DISABLED");
      break;
    case SUPLA_HVAC_AUX_THERMOMETER_TYPE_FLOOR:
      snprintf(enum_name, sizeof(enum_name), "FLOOR");
      break;
    case SUPLA_HVAC_AUX_THERMOMETER_TYPE_WATER:
      snprintf(enum_name, sizeof(enum_name), "WATER");
      break;
    case SUPLA_HVAC_AUX_THERMOMETER_TYPE_GENERIC_HEATER:
      snprintf(enum_name, sizeof(enum_name), "GENERIC_HEATER");
      break;
    case SUPLA_HVAC_AUX_THERMOMETER_TYPE_GENERIC_COOLER:
      snprintf(enum_name, sizeof(enum_name), "GENERIC_COOLER");
      break;
    default:
      snprintf(enum_name, sizeof(enum_name), "NOT_SET");
      break;
  }

  return supla_NewEnum(
      env, "org/supla/android/data/source/remote/hvac/SuplaHvacThermometerType",
      enum_name);
}

jobject supla_cc_hvac_algorithm_to_jobject(JNIEnv *env,
                                           unsigned char algorithm) {
  char enum_name[10] = {};

  switch (algorithm) {
    case SUPLA_HVAC_ALGORITHM_ON_OFF:
      snprintf(enum_name, sizeof(enum_name), "ON_OFF");
      break;
    default:
      snprintf(enum_name, sizeof(enum_name), "NOT_SET");
      break;
  }

  return supla_NewEnum(
      env, "org/supla/android/data/source/remote/hvac/SuplaHvacAlgorithm",
      enum_name);
}

jobject supla_cc_hvac_avil_algs_to_jobject(
    JNIEnv *env, unsigned _supla_int16_t available_algorithms) {
  jobject jarr = supla_NewArrayList(env);

  if (available_algorithms & SUPLA_HVAC_ALGORITHM_ON_OFF) {
    jobject alg =
        supla_cc_hvac_algorithm_to_jobject(env, SUPLA_HVAC_ALGORITHM_ON_OFF);
    supla_AddItemToArrayList(env, jarr, alg);
  }

  return jarr;
}

jobject supla_cc_vhac_get_temperature_jobject(JNIEnv *env,
                                              THVACTemperatureCfg *temperatures,
                                              unsigned _supla_int_t index) {
  if (temperatures->Index & index) {
    unsigned int size =
        sizeof(temperatures->Temperature) / sizeof(_supla_int16_t);
    for (int a = 0; a < size; a++) {
      if (1 << a == index) {
        return supla_NewDouble(env, temperatures->Temperature[a] * 0.01);
      }
    }
  }

  return env->NewGlobalRef(nullptr);
}

jobject supla_cc_vhac_temperatures_to_jobject(
    JNIEnv *env, THVACTemperatureCfg *temperatures) {
  jobject freeze_protection = supla_cc_vhac_get_temperature_jobject(
      env, temperatures, TEMPERATURE_FREEZE_PROTECTION);

  jobject eco =
      supla_cc_vhac_get_temperature_jobject(env, temperatures, TEMPERATURE_ECO);

  jobject comfort = supla_cc_vhac_get_temperature_jobject(env, temperatures,
                                                          TEMPERATURE_COMFORT);

  jobject boost = supla_cc_vhac_get_temperature_jobject(env, temperatures,
                                                        TEMPERATURE_BOOST);

  jobject heat_protection = supla_cc_vhac_get_temperature_jobject(
      env, temperatures, TEMPERATURE_HEAT_PROTECTION);

  jobject hysteresis = supla_cc_vhac_get_temperature_jobject(
      env, temperatures, TEMPERATURE_HISTERESIS);

  jobject below_alarm = supla_cc_vhac_get_temperature_jobject(
      env, temperatures, TEMPERATURE_BELOW_ALARM);

  jobject above_alarm = supla_cc_vhac_get_temperature_jobject(
      env, temperatures, TEMPERATURE_ABOVE_ALARM);

  jobject aux_min_setpoint = supla_cc_vhac_get_temperature_jobject(
      env, temperatures, TEMPERATURE_AUX_MIN_SETPOINT);

  jobject aux_max_setpoint = supla_cc_vhac_get_temperature_jobject(
      env, temperatures, TEMPERATURE_AUX_MAX_SETPOINT);

  jobject room_min = supla_cc_vhac_get_temperature_jobject(
      env, temperatures, TEMPERATURE_ROOM_MIN);

  jobject room_max = supla_cc_vhac_get_temperature_jobject(
      env, temperatures, TEMPERATURE_ROOM_MAX);

  jobject aux_min = supla_cc_vhac_get_temperature_jobject(env, temperatures,
                                                          TEMPERATURE_AUX_MIN);

  jobject aux_max = supla_cc_vhac_get_temperature_jobject(env, temperatures,
                                                          TEMPERATURE_AUX_MAX);

  jobject histeresis_min = supla_cc_vhac_get_temperature_jobject(
      env, temperatures, TEMPERATURE_HISTERESIS_MIN);

  jobject histeresis_max = supla_cc_vhac_get_temperature_jobject(
      env, temperatures, TEMPERATURE_HISTERESIS_MAX);

  jobject auto_offset_min = supla_cc_vhac_get_temperature_jobject(
      env, temperatures, TEMPERATURE_AUTO_OFFSET_MIN);

  jobject auto_offset_max = supla_cc_vhac_get_temperature_jobject(
      env, temperatures, TEMPERATURE_AUTO_OFFSET_MAX);

  jclass temperatures_cls = env->FindClass(
      "org/supla/android/data/source/remote/hvac/SuplaHvacTemperatures");

  jmethodID method_init = env->GetMethodID(
      temperatures_cls, "<init>",
      "(Ljava/lang/Double;Ljava/lang/Double;Ljava/lang/Double;Ljava/lang/"
      "Double;Ljava/lang/Double;Ljava/lang/Double;Ljava/lang/Double;Ljava/lang/"
      "Double;Ljava/lang/Double;Ljava/lang/Double;Ljava/lang/Double;Ljava/lang/"
      "Double;Ljava/lang/Double;Ljava/lang/Double;Ljava/lang/Double;Ljava/lang/"
      "Double;Ljava/lang/Double;Ljava/lang/Double;)V");

  jobject result = env->NewObject(
      temperatures_cls, method_init, freeze_protection, eco, comfort, boost,
      heat_protection, hysteresis, below_alarm, above_alarm, aux_min_setpoint,
      aux_max_setpoint, room_min, room_max, aux_min, aux_max, histeresis_min,
      histeresis_max, auto_offset_min, auto_offset_max);

  env->DeleteLocalRef(temperatures_cls);

  return result;
}

jobject supla_cc_hvac_to_jobject(JNIEnv *env, _supla_int_t channel_id,
                                 _supla_int_t func, TChannelConfig_HVAC *hvac) {
  jclass config_cls = env->FindClass(
      "org/supla/android/data/source/remote/hvac/SuplaChannelHvacConfig");

  jmethodID method_init = env->GetMethodID(
      config_cls, "<init>",
      "(ILjava/lang/Integer;IILorg/supla/android/data/source/remote/hvac/"
      "SuplaHvacThermometerType;ZLjava/util/List;Lorg/supla/android/data/"
      "source/remote/hvac/SuplaHvacAlgorithm;IIILorg/supla/android/data/source/"
      "remote/hvac/SuplaHvacTemperatures;)V");

  jint min_on_time_s = hvac->MinOnTimeS;
  jint min_off_time_s = hvac->MinOffTimeS;
  jint output_value_on_error = hvac->OutputValueOnError;

  jobject aux_thermometer_type =
      supla_cc_hvac_thermometer_type_to_jobject(env, hvac->AuxThermometerType);

  jobject available_algorithms =
      supla_cc_hvac_avil_algs_to_jobject(env, hvac->AvailableAlgorithms);

  jobject used_algorithm =
      supla_cc_hvac_algorithm_to_jobject(env, hvac->UsedAlgorithm);

  jobject temperatures =
      supla_cc_vhac_temperatures_to_jobject(env, &hvac->Temperatures);

  jobject result = env->NewObject(
      config_cls, method_init, channel_id, supla_NewInt(env, func),
      (jint)hvac->MainThermometerChannelId, (jint)hvac->AuxThermometerChannelId,
      aux_thermometer_type,
      hvac->AntiFreezeAndOverheatProtectionEnabled ? JNI_TRUE : JNI_FALSE,
      available_algorithms, used_algorithm, min_on_time_s, min_off_time_s,
      output_value_on_error, temperatures);

  env->DeleteLocalRef(config_cls);

  return result;
}
