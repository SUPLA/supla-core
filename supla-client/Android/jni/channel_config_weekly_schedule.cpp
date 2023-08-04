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

#include "channel_config_weekly_schedule.h"

#include <stdlib.h>

#include "supla.h"

jobject supla_cc_hvac_mode_to_jobject(JNIEnv *env, unsigned char mode) {
  char enum_name[20] = {};

  switch (mode) {
    case SUPLA_HVAC_MODE_OFF:
      snprintf(enum_name, sizeof(enum_name), "OFF");
      break;
    case SUPLA_HVAC_MODE_HEAT:
      snprintf(enum_name, sizeof(enum_name), "HEAT");
      break;
    case SUPLA_HVAC_MODE_COOL:
      snprintf(enum_name, sizeof(enum_name), "COOL");
      break;
    case SUPLA_HVAC_MODE_AUTO:
      snprintf(enum_name, sizeof(enum_name), "AUTO");
      break;
    case SUPLA_HVAC_MODE_FAN_ONLY:
      snprintf(enum_name, sizeof(enum_name), "FAN_ONLY");
      break;
    case SUPLA_HVAC_MODE_DRY:
      snprintf(enum_name, sizeof(enum_name), "DRY");
      break;
    case SUPLA_HVAC_MODE_CMD_TURN_ON:
      snprintf(enum_name, sizeof(enum_name), "TURN_ON");
      break;
    case SUPLA_HVAC_MODE_CMD_WEEKLY_SCHEDULE:
      snprintf(enum_name, sizeof(enum_name), "WEEKLY_SCHEDULE");
      break;
    default:
      snprintf(enum_name, sizeof(enum_name), "NOT_SET");
      break;
  }

  return supla_NewEnum(
      env, "org/supla/android/data/source/remote/hvac/SuplaHvacMode", enum_name);
}

jobject supla_cc_schedule_program_to_jobject(JNIEnv *env,
                                             unsigned char program) {
  char enum_name[15] = {};

  if (program >= 1 && program <= 4) {
    snprintf(enum_name, sizeof(enum_name), "PROGRAM_%i", program);
  } else {
    snprintf(enum_name, sizeof(enum_name), "OFF");
  }

  return supla_NewEnum(
      env, "org/supla/android/data/source/remote/hvac/SuplaScheduleProgram",
      enum_name);
}

jobject supla_cc_ws_program_to_jobject(JNIEnv *env, unsigned char index,
                                       TWeeklyScheduleProgram *program) {
  jobject enum_program = supla_cc_schedule_program_to_jobject(env, index);

  jobject hvac_mode = supla_cc_hvac_mode_to_jobject(env, program->Mode);

  jdouble setpoint_temperature_min = program->SetpointTemperatureMin * 0.01;
  jdouble setpoint_temperature_max = program->SetpointTemperatureMax * 0.01;

  jclass program_cls = env->FindClass(
      "org/supla/android/data/source/remote/hvac/SuplaWeeklyScheduleProgram");

  jmethodID init_method = env->GetMethodID(
      program_cls, "<init>",
      "(Lorg/supla/android/data/source/remote/hvac/SuplaScheduleProgram;Lorg/"
      "supla/android/data/source/remote/hvac/SuplaHvacMode;DD)V");

  jobject result =
      env->NewObject(program_cls, init_method, enum_program, hvac_mode,
                     setpoint_temperature_min, setpoint_temperature_max);

  env->DeleteLocalRef(program_cls);

  return result;
}

jobject supla_cc_ws_program_configurations_to_jobject(
    JNIEnv *env, TChannelConfig_WeeklySchedule *ws) {
  jobject jarr = supla_NewArrayList(env);

  for (unsigned char a = 0; a < SUPLA_WEEKLY_SCHEDULE_PROGRAMS_MAX_SIZE; a++) {
    jobject program =
        supla_cc_ws_program_to_jobject(env, a + 1, &ws->Program[a]);
    supla_AddItemToArrayList(env, jarr, program);
  }

  return jarr;
}

jobject supla_cc_day_of_week_to_jobject(JNIEnv *env,
                                        unsigned char day_of_week) {
  char enum_name[20] = {};

  switch (day_of_week) {
    case 0:
      snprintf(enum_name, sizeof(enum_name), "SUNDAY");
      break;
    case 1:
      snprintf(enum_name, sizeof(enum_name), "MONDAY");
      break;
    case 2:
      snprintf(enum_name, sizeof(enum_name), "TUESDAY");
      break;
    case 3:
      snprintf(enum_name, sizeof(enum_name), "WEDNESDAY");
      break;
    case 4:
      snprintf(enum_name, sizeof(enum_name), "THURSDAY");
      break;
    case 5:
      snprintf(enum_name, sizeof(enum_name), "FRIDAY");
      break;
    case 6:
      snprintf(enum_name, sizeof(enum_name), "SATURDAY");
      break;
  }

  return supla_NewEnum(env, "java/time/DayOfWeek", enum_name);
}

jobject supla_cc_ws_get_entry(JNIEnv *env, unsigned char day_of_week,
                              unsigned char hour, unsigned char quarter_of_hour,
                              unsigned char program_index) {
  jclass entry_cls = env->FindClass(
      "org/supla/android/data/source/remote/hvac/SuplaWeeklyScheduleEntry");

  jmethodID init_method =
      env->GetMethodID(entry_cls, "<init>",
                       "(Ljava/time/DayOfWeek;IILorg/supla/android/data/source/"
                       "remote/hvac/SuplaScheduleProgram;)V");

  jobject jday_of_week = supla_cc_day_of_week_to_jobject(env, day_of_week);
  jint jhour = hour;
  jint jquarter_of_hour = quarter_of_hour;
  jobject program = supla_cc_schedule_program_to_jobject(env, program_index);

  jobject result = env->NewObject(entry_cls, init_method, jday_of_week, jhour,
                                  jquarter_of_hour, program);

  env->DeleteLocalRef(entry_cls);

  return result;
}

jobject supla_cc_ws_quarters_to_jobject(JNIEnv *env,
                                        TChannelConfig_WeeklySchedule *ws) {
  jobject jarr = supla_NewArrayList(env);

  for (unsigned int a = 0; a < sizeof(ws->Quarters); a++) {
    unsigned char day_of_week = (a / 2 / 24) % 7;
    unsigned char hour = (a / 2) % 24;
    unsigned char quarter_of_hour = (a % 2) * 2;

    jobject entry = supla_cc_ws_get_entry(
        env, day_of_week, hour, quarter_of_hour + 1, ws->Quarters[a] & 0xF);

    supla_AddItemToArrayList(env, jarr, entry);

    entry = supla_cc_ws_get_entry(env, day_of_week, hour, quarter_of_hour + 2,
                                  (ws->Quarters[a] & 0xF0) >> 4);

    supla_AddItemToArrayList(env, jarr, entry);
  }
  return jarr;
}

jobject supla_cc_weekly_schedule_to_jobject(JNIEnv *env,
                                            _supla_int_t channel_id,
                                            _supla_int_t func,
                                            TChannelConfig_WeeklySchedule *ws) {
  jclass config_cls = env->FindClass(
      "org/supla/android/data/source/remote/hvac/"
      "SuplaChannelWeeklyScheduleConfig");

  jmethodID method_init = env->GetMethodID(
      config_cls, "<init>", "(IILjava/util/List;Ljava/util/List;)V");

  jobject program_configurations =
      supla_cc_ws_program_configurations_to_jobject(env, ws);
  jobject schedule = supla_cc_ws_quarters_to_jobject(env, ws);

  jobject result = env->NewObject(config_cls, method_init, channel_id, func,
                                  program_configurations, schedule);

  env->DeleteLocalRef(config_cls);

  return result;
}
