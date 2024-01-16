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

#include "channel_config_general_purpose_measurement.h"

#include <stdlib.h>

#include "supla.h"

jobject supla_cc_gp_measurement_char_type_to_object(JNIEnv *env,
                                                    unsigned char chart_type) {
  char enum_name[20] = {};

  switch (chart_type) {
    case SUPLA_GENERAL_PURPOSE_MEASUREMENT_CHART_TYPE_BAR:
      snprintf(enum_name, sizeof(enum_name), "BAR");
      break;
    case SUPLA_GENERAL_PURPOSE_MEASUREMENT_CHART_TYPE_CANDLE:
      snprintf(enum_name, sizeof(enum_name), "CANDLE");
      break;
    default:
      snprintf(enum_name, sizeof(enum_name), "LINEAR");
      break;
  }

  return supla_NewEnum(env,
                       "org/supla/android/data/source/remote/gpm/"
                       "SuplaChannelConfigMeasurementChartType",
                       enum_name);
}

jobject supla_cc_gp_measurement_to_jobject(
    JNIEnv *env, _supla_int_t channel_id, _supla_int_t func,
    TChannelConfig_GeneralPurposeMeasurement *config) {
  jclass config_cls = env->FindClass(
      "org/supla/android/data/source/remote/gpm/"
      "SuplaChannelGeneralPurposeMeasurementConfig");

  jmethodID method_init = env->GetMethodID(
      config_cls, "<init>",
      "(ILjava/lang/Integer;IIJILjava/lang/String;Ljava/lang/"
      "String;ZZIIJILjava/lang/String;Ljava/lang/String;Lorg/supla/android/"
      "data/source/remote/gpm/SuplaChannelConfigMeasurementChartType;)V");

  config->UnitBeforeValue[sizeof(config->UnitBeforeValue) - 1] = 0;
  jobject unit_before_value = new_string_utf(env, config->UnitBeforeValue);

  config->UnitAfterValue[sizeof(config->UnitAfterValue) - 1] = 0;
  jobject unit_after_value = new_string_utf(env, config->UnitAfterValue);

  config->DefaultUnitBeforeValue[sizeof(config->DefaultUnitBeforeValue) - 1] =
      0;
  jobject default_unit_before_value =
      new_string_utf(env, config->DefaultUnitBeforeValue);

  config->DefaultUnitAfterValue[sizeof(config->DefaultUnitAfterValue) - 1] = 0;
  jobject default_unit_after_value =
      new_string_utf(env, config->DefaultUnitAfterValue);

  jobject chart_type =
      supla_cc_gp_measurement_char_type_to_object(env, config->ChartType);

  jobject result = env->NewObject(
      config_cls, method_init, channel_id, supla_NewInt(env, func),
      (jint)config->ValueDivider, (jint)config->ValueMultiplier,
      (jlong)config->ValueAdded, (jint)config->ValuePrecision,
      unit_before_value, unit_after_value,
      config->NoSpaceAfterValue ? JNI_TRUE : JNI_FALSE,
      config->KeepHistory ? JNI_TRUE : JNI_FALSE,
      (jint)config->DefaultValueDivider, (jint)config->DefaultValueMultiplier,
      (jlong)config->DefaultValueAdded, (jint)config->DefaultValuePrecision,
      default_unit_before_value, default_unit_after_value, chart_type);

  env->DeleteLocalRef(config_cls);

  return result;
}
