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

#include <android/log.h>
#include <cstdio>
#include <stdlib.h>

#include "supla.h"

jobject supla_cc_hvac_mode_to_jobject(JNIEnv *env, unsigned char mode) {
  char enum_name[30] = {};

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
    case SUPLA_HVAC_MODE_HEAT_COOL:
      snprintf(enum_name, sizeof(enum_name), "HEAT_COOL");
      break;
    case SUPLA_HVAC_MODE_FAN_ONLY:
      snprintf(enum_name, sizeof(enum_name), "FAN_ONLY");
      break;
    case SUPLA_HVAC_MODE_DRY:
      snprintf(enum_name, sizeof(enum_name), "DRY");
      break;
    case SUPLA_HVAC_MODE_CMD_TURN_ON:
      snprintf(enum_name, sizeof(enum_name), " CMD_TURN_ON");
      break;
    case SUPLA_HVAC_MODE_CMD_WEEKLY_SCHEDULE:
      snprintf(enum_name, sizeof(enum_name), " CMD_WEEKLY_SCHEDULE");
      break;
    case SUPLA_HVAC_MODE_CMD_SWITCH_TO_MANUAL:
      snprintf(enum_name, sizeof(enum_name), " CMD_SWITCH_TO_MANUAL");
      break;
    default:
      snprintf(enum_name, sizeof(enum_name), "NOT_SET");
      break;
  }

  return supla_NewEnum(
      env, "org/supla/android/data/source/remote/hvac/SuplaHvacMode",
      enum_name);
}

jobject supla_cc_relay_mode_to_jobject(JNIEnv *env, unsigned char mode) {
  char enum_name[30] = {};

  switch (mode) {
    case SUPLA_RELAY_MODE_START_ON:
      snprintf(enum_name, sizeof(enum_name), "START_ON");
      break;
    case SUPLA_RELAY_MODE_START_OFF:
      snprintf(enum_name, sizeof(enum_name), "START_OFF");
      break;
    case SUPLA_RELAY_MODE_FORCED_ON:
      snprintf(enum_name, sizeof(enum_name), "FORCED_ON");
      break;
    case SUPLA_RELAY_MODE_FORCED_OFF:
      snprintf(enum_name, sizeof(enum_name), "FORCED_OFF");
      break;
    case SUPLA_RELAY_MODE_AUTOMATIC:
      snprintf(enum_name, sizeof(enum_name), "AUTOMATIC");
      break;
    case SUPLA_RELAY_MODE_CMD_WEEKLY_SCHEDULE:
      snprintf(enum_name, sizeof(enum_name), "CMD_WEEKLY_SCHEDULE");
      break;
    case SUPLA_RELAY_MODE_CMD_SWITCH_TO_MANUAL:
      snprintf(enum_name, sizeof(enum_name), "CMD_SWITCH_TO_MANUAL");
      break;
    default:
      snprintf(enum_name, sizeof(enum_name), "NOT_SET");
      break;
  }

  return supla_NewEnum(
      env, "org/supla/android/data/source/remote/hvac/SuplaRelayMode",
      enum_name);
}

jobject supla_cc_button_mode_to_jobject(JNIEnv *env, unsigned char mode) {
  char enum_name[30] = {};

  switch (mode) {
    case SUPLA_BUTTON_MODE_LOCKED:
      snprintf(enum_name, sizeof(enum_name), "LOCKED");
      break;
    case SUPLA_BUTTON_MODE_CMD_WEEKLY_SCHEDULE:
      snprintf(enum_name, sizeof(enum_name), "CMD_WEEKLY_SCHEDULE");
      break;
    case SUPLA_BUTTON_MODE_CMD_SWITCH_TO_MANUAL:
      snprintf(enum_name, sizeof(enum_name), "CMD_SWITCH_TO_MANUAL");
      break;
    default:
      snprintf(enum_name, sizeof(enum_name), "NOT_SET");
      break;
  }

  return supla_NewEnum(
      env, "org/supla/android/data/source/remote/hvac/SuplaButtonMode",
      enum_name);
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
                                       _supla_int_t func,
                                       TWeeklyScheduleProgram *program) {
  jobject enum_program = supla_cc_schedule_program_to_jobject(env, index);
  jobject hvac_mode = nullptr;
  jobject setpoint_temperature_heat = nullptr;
  jobject setpoint_temperature_cool = nullptr;
  jobject relay_mode = nullptr;
  jobject relay_mode_duration_s = nullptr;
  jobject relay_opposite_mode_duration_s = nullptr;
  jobject button_mode = nullptr;

  switch (supla_weekly_schedule_get_program_mode_type(func)) {
    case SUPLA_WEEKLY_SCHEDULE_PROGRAM_MODE_TYPE_RELAY:
      relay_mode = supla_cc_relay_mode_to_jobject(env, program->Mode);
      relay_mode_duration_s =
          supla_NewInt(env, program->RelayModeDurationS);
      relay_opposite_mode_duration_s =
          supla_NewInt(env, program->RelayOppositeModeDurationS);
      break;
    case SUPLA_WEEKLY_SCHEDULE_PROGRAM_MODE_TYPE_BUTTON:
      button_mode = supla_cc_button_mode_to_jobject(env, program->Mode);
      break;
    default:
      hvac_mode = supla_cc_hvac_mode_to_jobject(env, program->Mode);
      setpoint_temperature_heat =
          supla_NewShort(env, program->SetpointTemperatureHeat);
      setpoint_temperature_cool =
          supla_NewShort(env, program->SetpointTemperatureCool);
      break;
  }

  jclass program_cls = env->FindClass(
      "org/supla/android/data/source/remote/hvac/SuplaWeeklyScheduleProgram");

  jmethodID init_method = env->GetMethodID(
      program_cls, "<init>",
      "(Lorg/supla/android/data/source/remote/hvac/SuplaScheduleProgram;Lorg/"
      "supla/android/data/source/remote/hvac/SuplaHvacMode;Ljava/lang/"
      "Short;Ljava/lang/Short;Lorg/supla/android/data/source/remote/hvac/"
      "SuplaRelayMode;Ljava/lang/Integer;Ljava/lang/Integer;Lorg/supla/"
      "android/data/source/remote/hvac/SuplaButtonMode;)V");

  jobject result = env->NewObject(
      program_cls, init_method, enum_program, hvac_mode,
      setpoint_temperature_heat, setpoint_temperature_cool, relay_mode,
      relay_mode_duration_s, relay_opposite_mode_duration_s, button_mode);

  env->DeleteLocalRef(program_cls);
  env->DeleteLocalRef(enum_program);

  if (hvac_mode) {
    env->DeleteLocalRef(hvac_mode);
    env->DeleteLocalRef(setpoint_temperature_heat);
    env->DeleteLocalRef(setpoint_temperature_cool);
  }

  if (relay_mode) {
    env->DeleteLocalRef(relay_mode);
    env->DeleteLocalRef(relay_mode_duration_s);
    env->DeleteLocalRef(relay_opposite_mode_duration_s);
  }

  if (button_mode) {
    env->DeleteLocalRef(button_mode);
  }

  return result;
}

jobject supla_cc_ws_program_configurations_to_jobject(
    JNIEnv *env, _supla_int_t func, TChannelConfig_WeeklySchedule *ws) {
  jobject jarr = supla_NewArrayList(env);

  for (unsigned char a = 0; a < SUPLA_WEEKLY_SCHEDULE_PROGRAMS_MAX_SIZE; a++) {
    jobject program =
        supla_cc_ws_program_to_jobject(env, a + 1, func, &ws->Program[a]);
    supla_AddItemToArrayList(env, jarr, program);
    env->DeleteLocalRef(program);
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

  return supla_NewEnum(
      env, "org/supla/android/data/source/local/calendar/DayOfWeek", enum_name);
}

jobject supla_cc_quarter_of_hour_to_jobject(JNIEnv *env,
                                            unsigned char quarter_of_hour) {
  char enum_name[20] = {};

  switch (quarter_of_hour) {
    case 1:
      snprintf(enum_name, sizeof(enum_name), "FIRST");
      break;
    case 2:
      snprintf(enum_name, sizeof(enum_name), "SECOND");
      break;
    case 3:
      snprintf(enum_name, sizeof(enum_name), "THIRD");
      break;
    case 4:
      snprintf(enum_name, sizeof(enum_name), "FOURTH");
      break;
  }

  return supla_NewEnum(
      env, "org/supla/android/data/source/local/calendar/QuarterOfHour",
      enum_name);
}

jobject supla_cc_ws_get_entry(JNIEnv *env, unsigned char day_of_week,
                              unsigned char hour, unsigned char quarter_of_hour,
                              unsigned char program_index) {
  jclass entry_cls = env->FindClass(
      "org/supla/android/data/source/remote/hvac/SuplaWeeklyScheduleEntry");

  jmethodID init_method = env->GetMethodID(
      entry_cls, "<init>",
      "(Lorg/supla/android/data/source/local/calendar/DayOfWeek;ILorg/supla/"
      "android/data/source/local/calendar/QuarterOfHour;Lorg/supla/android/"
      "data/source/remote/hvac/SuplaScheduleProgram;)V");

  jobject jday_of_week = supla_cc_day_of_week_to_jobject(env, day_of_week);
  jint jhour = hour;
  jobject jquarter_of_hour =
      supla_cc_quarter_of_hour_to_jobject(env, quarter_of_hour);
  jobject program = supla_cc_schedule_program_to_jobject(env, program_index);

  jobject result = env->NewObject(entry_cls, init_method, jday_of_week, jhour,
                                  jquarter_of_hour, program);

  env->DeleteLocalRef(entry_cls);
  env->DeleteLocalRef(jday_of_week);
  env->DeleteLocalRef(jquarter_of_hour);
  env->DeleteLocalRef(program);

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
    env->DeleteLocalRef(entry);

    entry = supla_cc_ws_get_entry(env, day_of_week, hour, quarter_of_hour + 2,
                                  (ws->Quarters[a] & 0xF0) >> 4);

    supla_AddItemToArrayList(env, jarr, entry);
    env->DeleteLocalRef(entry);
  }
  return jarr;
}

jobject supla_cc_weekly_schedule_to_jobject(JNIEnv *env,
                                            _supla_int_t channel_id,
                                            _supla_int_t func, jlong crc32,
                                            TChannelConfig_WeeklySchedule *ws) {
  jclass config_cls = env->FindClass(
      "org/supla/android/data/source/remote/hvac/"
      "SuplaChannelWeeklyScheduleConfig");

  jmethodID method_init = env->GetMethodID(
      config_cls, "<init>",
      "(ILjava/lang/Integer;JLjava/util/List;Ljava/util/List;)V");

  jobject program_configurations =
      supla_cc_ws_program_configurations_to_jobject(env, func, ws);
  jobject schedule = supla_cc_ws_quarters_to_jobject(env, ws);

  jobject result = env->NewObject(config_cls, method_init, channel_id,
                                  supla_NewInt(env, func), crc32,
                                  program_configurations, schedule);

  env->DeleteLocalRef(config_cls);
  env->DeleteLocalRef(program_configurations);
  env->DeleteLocalRef(schedule);

  return result;
}

static bool supla_cc_ws_get_program_mode(
    JNIEnv *env, jclass program_cls, jobject item, const char *getter,
    const char *signature, const char *enum_class, unsigned char *mode) {
  jobject mode_enum =
      supla_CallObjectMethod(env, program_cls, item, getter, signature);
  if (env->IsSameObject(mode_enum, nullptr)) {
    return false;
  }

  jint value = supla_GetEnumValue(env, mode_enum, enum_class);
  env->DeleteLocalRef(mode_enum);

  if (value < 0 || value > 255) {
    return false;
  }

  *mode = value;
  return true;
}

static unsigned _supla_int16_t supla_cc_ws_get_duration_s(
    JNIEnv *env, jclass program_cls, jobject item, const char *getter) {
  jint result = 0;
  if (!supla_CallIntObjectMethod(env, program_cls, item, getter, &result) ||
      result < 0 || result > 65535) {
    return 0;
  }

  return result;
}

static void supla_cc_ws_get_hvac_program(JNIEnv *env, jclass program_cls,
                                         jobject item,
                                         TWeeklyScheduleProgram *program) {
  supla_cc_ws_get_program_mode(
      env, program_cls, item, "getMode",
      "()Lorg/supla/android/data/source/remote/hvac/SuplaHvacMode;",
      "org/supla/android/data/source/remote/hvac/SuplaHvacMode",
      &program->Mode);

  jshort value = 0;
  supla_CallShortObjectMethod(env, program_cls, item,
                              "getSetpointTemperatureHeat", &value);
  program->SetpointTemperatureHeat = value;

  value = 0;
  supla_CallShortObjectMethod(env, program_cls, item,
                              "getSetpointTemperatureCool", &value);
  program->SetpointTemperatureCool = value;
}

static void supla_cc_ws_get_relay_program(JNIEnv *env, jclass program_cls,
                                          jobject item,
                                          TWeeklyScheduleProgram *program) {
  program->RelayModeDurationS = supla_cc_ws_get_duration_s(
      env, program_cls, item, "getRelayModeDurationS");
  program->RelayOppositeModeDurationS = supla_cc_ws_get_duration_s(
      env, program_cls, item, "getRelayOppositeModeDurationS");
}

static void supla_cc_ws_get_button_program(TWeeklyScheduleProgram *program) {
  program->Value1 = 0;
  program->Value2 = 0;
}

void supla_cc_ws_get_programs(JNIEnv *env, jobject programs,
                              TChannelConfig_WeeklySchedule *ws) {
  int prog_size = supla_GetListSize(env, programs);
  jclass program_cls = env->FindClass(
      "org/supla/android/data/source/remote/hvac/SuplaWeeklyScheduleProgram");

  for (int a = 0; a < prog_size; a++) {
    jobject item = supla_GetListItem(env, programs, a);

    jobject program_enum = supla_CallObjectMethod(
        env, program_cls, item, "getProgram",
        "()Lorg/supla/android/data/source/remote/hvac/SuplaScheduleProgram;");

    jint program_id = supla_GetEnumValue(
        env, program_enum,
        "org/supla/android/data/source/remote/hvac/SuplaScheduleProgram");

    if (program_id > 0 && program_id < 5) {
      TWeeklyScheduleProgram *program = &ws->Program[program_id - 1];
      if (supla_cc_ws_get_program_mode(
              env, program_cls, item, "getRelayMode",
              "()Lorg/supla/android/data/source/remote/hvac/SuplaRelayMode;",
              "org/supla/android/data/source/remote/hvac/SuplaRelayMode",
              &program->Mode)) {
        supla_cc_ws_get_relay_program(env, program_cls, item, program);
      } else if (supla_cc_ws_get_program_mode(
                     env, program_cls, item, "getButtonMode",
                     "()Lorg/supla/android/data/source/remote/hvac/"
                     "SuplaButtonMode;",
                     "org/supla/android/data/source/remote/hvac/"
                     "SuplaButtonMode",
                     &program->Mode)) {
        supla_cc_ws_get_button_program(program);
      } else {
        supla_cc_ws_get_hvac_program(env, program_cls, item, program);
      }
    }

    env->DeleteLocalRef(item);
    env->DeleteLocalRef(program_enum);
  }

  env->DeleteLocalRef(program_cls);
}

void supla_cc_ws_get_quarters(JNIEnv *env, jobject schedule,
                              TChannelConfig_WeeklySchedule *ws) {
  int schedule_size = supla_GetListSize(env, schedule);

  jclass schedule_entry_cls = env->FindClass(
      "org/supla/android/data/source/remote/hvac/SuplaWeeklyScheduleEntry");

  for (int a = 0; a < schedule_size; a++) {
    jobject item = supla_GetListItem(env, schedule, a);

    jobject day_of_week_enum = supla_CallObjectMethod(
        env, schedule_entry_cls, item, "getDayOfWeek",
        "()Lorg/supla/android/data/source/local/calendar/DayOfWeek;");

    jint day_of_week = supla_GetEnumValue(
        env, day_of_week_enum,
        "org/supla/android/data/source/local/calendar/DayOfWeek", "getDay");

    if (day_of_week == 7) {
      day_of_week = 0;
    }

    jobject program_enum = supla_CallObjectMethod(
        env, schedule_entry_cls, item, "getProgram",
        "()Lorg/supla/android/data/source/remote/hvac/SuplaScheduleProgram;");

    jint program = supla_GetEnumValue(
        env, program_enum,
        "org/supla/android/data/source/remote/hvac/SuplaScheduleProgram");

    jint hour = supla_CallIntMethod(env, schedule_entry_cls, item, "getHour");
    jobject quarter_of_hour_enum = supla_CallObjectMethod(
        env, schedule_entry_cls, item, "getQuarterOfHour",
        "()Lorg/supla/android/data/source/local/calendar/QuarterOfHour;");

    jint quarter_of_hour = supla_GetEnumValue(
        env, quarter_of_hour_enum,
        "org/supla/android/data/source/local/calendar/QuarterOfHour",
        "getStartingMinute");

    switch (quarter_of_hour) {
      case 15:
        quarter_of_hour = 2;
        break;
      case 30:
        quarter_of_hour = 3;
        break;
      case 45:
        quarter_of_hour = 4;
        break;
      default:
        quarter_of_hour = 1;
    }

    if (day_of_week >= 0 && day_of_week <= 6 && hour >= 0 && hour <= 23 &&
        quarter_of_hour >= 1 && quarter_of_hour <= 4 && program >= 0 &&
        program <= 4) {
      int index = (day_of_week * 24 + hour) * 2 + quarter_of_hour / 3;

      if (index < sizeof(ws->Quarters)) {
        unsigned char q = program & 0xF;
        if (quarter_of_hour == 2 || quarter_of_hour == 4) {
          q <<= 4;
        }
        ws->Quarters[index] |= q;
      }
    }

    env->DeleteLocalRef(item);
    env->DeleteLocalRef(day_of_week_enum);
    env->DeleteLocalRef(program_enum);
    env->DeleteLocalRef(quarter_of_hour_enum);
  }

  env->DeleteLocalRef(schedule_entry_cls);
}

bool supla_cc_jobject_to_weekly_schedule(JNIEnv *env, jobject object,
                                         TSCS_ChannelConfig *config) {
  jclass cls = env->FindClass(
      "org/supla/android/data/source/remote/hvac/"
      "SuplaChannelWeeklyScheduleConfig");

  if (!env->IsInstanceOf(object, cls)) {
    env->DeleteLocalRef(cls);
    return false;
  }

  jmethodID method_program_list_getter =
      env->GetMethodID(cls, "getProgramConfigurations", "()Ljava/util/List;");

  jobject programs = env->CallObjectMethod(object, method_program_list_getter);

  jmethodID method_schedule_list_getter =
      env->GetMethodID(cls, "getSchedule", "()Ljava/util/List;");

  jobject schedule = env->CallObjectMethod(object, method_schedule_list_getter);

  TChannelConfig_WeeklySchedule *ws =
      (TChannelConfig_WeeklySchedule *)config->Config;

  supla_cc_ws_get_programs(env, programs, ws);
  supla_cc_ws_get_quarters(env, schedule, ws);

  env->DeleteLocalRef(cls);
  env->DeleteLocalRef(programs);
  env->DeleteLocalRef(schedule);

  config->ConfigType = SUPLA_CONFIG_TYPE_WEEKLY_SCHEDULE;
  config->ConfigSize = sizeof(TChannelConfig_WeeklySchedule);

  return true;
}
