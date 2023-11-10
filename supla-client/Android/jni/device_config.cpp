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

#include "device_config.h"

#include <android/log.h>
#include <stdlib.h>

#include "channel_config.h"

unsigned _supla_int64_t all_device_config_fields[] = {
    SUPLA_DEVICE_CONFIG_FIELD_STATUS_LED,
    SUPLA_DEVICE_CONFIG_FIELD_SCREEN_BRIGHTNESS,
    SUPLA_DEVICE_CONFIG_FIELD_BUTTON_VOLUME,
    SUPLA_DEVICE_CONFIG_FIELD_DISABLE_USER_INTERFACE,
    SUPLA_DEVICE_CONFIG_FIELD_AUTOMATIC_TIME_SYNC,
    SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_OFF_DELAY,
    SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_CONTENT};

unsigned _supla_int64_t all_content_types[] = {
    SUPLA_DEVCFG_HOME_SCREEN_CONTENT_NONE,
    SUPLA_DEVCFG_HOME_SCREEN_CONTENT_TEMPERATURE,
    SUPLA_DEVCFG_HOME_SCREEN_CONTENT_TEMPERATURE_AND_HUMIDITY,
    SUPLA_DEVCFG_HOME_SCREEN_CONTENT_TIME,
    SUPLA_DEVCFG_HOME_SCREEN_CONTENT_TIME_DATE,
    SUPLA_DEVCFG_HOME_SCREEN_CONTENT_TEMPERATURE_TIME,
    SUPLA_DEVCFG_HOME_SCREEN_CONTENT_MAIN_AND_AUX_TEMPERATURE};

jobject supla_device_config_field_to_object(JNIEnv *env,
                                            unsigned _supla_int64_t field) {
  const char cls_name[] = "org/supla/android/data/source/remote/FieldType";

  switch (field) {
    case SUPLA_DEVICE_CONFIG_FIELD_STATUS_LED:
      return supla_NewEnum(env, cls_name, "STATUS_LED");
    case SUPLA_DEVICE_CONFIG_FIELD_SCREEN_BRIGHTNESS:
      return supla_NewEnum(env, cls_name, "SCREEN_BRIGHTNESS");
    case SUPLA_DEVICE_CONFIG_FIELD_BUTTON_VOLUME:
      return supla_NewEnum(env, cls_name, "BUTTON_VOLUME");
    case SUPLA_DEVICE_CONFIG_FIELD_DISABLE_USER_INTERFACE:
      return supla_NewEnum(env, cls_name, "DISABLE_USER_INTERFACE");
    case SUPLA_DEVICE_CONFIG_FIELD_AUTOMATIC_TIME_SYNC:
      return supla_NewEnum(env, cls_name, "AUTOMATIC_TIME_SYNC");
    case SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_OFF_DELAY:
      return supla_NewEnum(env, cls_name, "HOME_SCREEN_OFF_DELAY");
    case SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_CONTENT:
      return supla_NewEnum(env, cls_name, "HOME_SCREEN_CONTENT");
  }

  return env->NewGlobalRef(nullptr);
}

jobject status_led_to_jobject(JNIEnv *env, jobject jfield_type,
                              TDeviceConfig_StatusLed *status_led) {
  jobject jstatus_led_type = env->NewGlobalRef(nullptr);

  const char status_led_type_cls_name[] =
      "org/supla/android/data/source/remote/StatusLedType";

  switch (status_led->StatusLedType) {
    case SUPLA_DEVCFG_STATUS_LED_ON_WHEN_CONNECTED:
      jstatus_led_type =
          supla_NewEnum(env, status_led_type_cls_name, "ON_WHEN_CONNECTED");
      break;
    case SUPLA_DEVCFG_STATUS_LED_OFF_WHEN_CONNECTED:
      jstatus_led_type =
          supla_NewEnum(env, status_led_type_cls_name, "OFF_WHEN_CONNECTED");
      break;
    case SUPLA_DEVCFG_STATUS_LED_ALWAYS_OFF:
      jstatus_led_type =
          supla_NewEnum(env, status_led_type_cls_name, "ALWAYS_OFF");
      break;
  }

  jclass cls =
      env->FindClass("org/supla/android/data/source/remote/StatusLedField");

  jmethodID init_method =
      env->GetMethodID(cls, "<init>",
                       "(Lorg/supla/android/data/source/remote/FieldType;Lorg/"
                       "supla/android/data/source/remote/StatusLedType;)V");

  return env->NewObject(cls, init_method, jfield_type, jstatus_led_type);
};

jobject screen_brightness_to_jobject(
    JNIEnv *env, jobject jfield_type,
    TDeviceConfig_ScreenBrightness *brightness) {
  jclass cls = env->FindClass(
      "org/supla/android/data/source/remote/ScreenBrightnessField");

  jmethodID init_method = env->GetMethodID(
      cls, "<init>", "(Lorg/supla/android/data/source/remote/FieldType;ZBB)V");

  return env->NewObject(cls, init_method, jfield_type,
                        brightness->Automatic ? JNI_TRUE : JNI_FALSE,
                        brightness->ScreenBrightness,
                        brightness->AdjustmentForAutomatic);
};

jobject button_volume_to_jobject(JNIEnv *env, jobject jfield_type,
                                 TDeviceConfig_ButtonVolume *volume) {
  jclass cls =
      env->FindClass("org/supla/android/data/source/remote/ButtonVolumeField");

  jmethodID init_method = env->GetMethodID(
      cls, "<init>", "(Lorg/supla/android/data/source/remote/FieldType;B)V");

  return env->NewObject(cls, init_method, jfield_type, volume->Volume);
};

jobject disable_user_interface_to_jobject(
    JNIEnv *env, jobject jfield_type, TDeviceConfig_DisableUserInterface *ui) {
  jobject jui_disabled = env->NewGlobalRef(nullptr);

  const char ui_disabled_cls_name[] =
      "org/supla/android/data/source/remote/UIDisabled";

  switch (ui->DisableUserInterface) {
    case 0:
      jui_disabled = supla_NewEnum(env, ui_disabled_cls_name, "NO");
      break;
    case 1:
      jui_disabled = supla_NewEnum(env, ui_disabled_cls_name, "YES");
      break;
    case 2:
      jui_disabled = supla_NewEnum(env, ui_disabled_cls_name, "PARTIAL");
      break;
  }

  jclass cls = env->FindClass(
      "org/supla/android/data/source/remote/DisableUserInterfaceField");

  jmethodID init_method =
      env->GetMethodID(cls, "<init>",
                       "(Lorg/supla/android/data/source/remote/FieldType;Lorg/"
                       "supla/android/data/source/remote/UIDisabled;SS)V");

  return env->NewObject(cls, init_method, jfield_type, jui_disabled,
                        ui->minAllowedTemperatureSetpointFromLocalUI,
                        ui->maxAllowedTemperatureSetpointFromLocalUI);
};

jobject automatic_time_sync_to_jobject(
    JNIEnv *env, jobject jfield_type,
    TDeviceConfig_AutomaticTimeSync *time_sync) {
  jclass cls = env->FindClass(
      "org/supla/android/data/source/remote/AutomaticTimeSyncField");

  jmethodID init_method = env->GetMethodID(
      cls, "<init>", "(Lorg/supla/android/data/source/remote/FieldType;Z)V");

  return env->NewObject(cls, init_method, jfield_type,
                        time_sync->AutomaticTimeSync ? JNI_TRUE : JNI_FALSE);
};

jobject home_screen_to_off_delay(JNIEnv *env, jobject jfield_type,
                                 TDeviceConfig_HomeScreenOffDelay *delay) {
  jclass cls = env->FindClass(
      "org/supla/android/data/source/remote/HomeScreenOffDelayField");

  jmethodID init_method = env->GetMethodID(
      cls, "<init>", "(Lorg/supla/android/data/source/remote/FieldType;ZI)V");

  jint delay_s = delay->HomeScreenOffDelayS;

  return env->NewObject(cls, init_method, jfield_type,
                        delay->HomeScreenOffDelayS > 0 ? JNI_TRUE : JNI_FALSE,
                        delay_s);
};

jobject home_screen_content_to_jobject(JNIEnv *env,
                                       unsigned _supla_int64_t content) {
  const char cls_name[] =
      "org/supla/android/data/source/remote/HomeScreenContent";

  switch (content) {
    case SUPLA_DEVCFG_HOME_SCREEN_CONTENT_NONE:
      return supla_NewEnum(env, cls_name, "NONE");
    case SUPLA_DEVCFG_HOME_SCREEN_CONTENT_TEMPERATURE:
      return supla_NewEnum(env, cls_name, "TEMPERATURE");
    case SUPLA_DEVCFG_HOME_SCREEN_CONTENT_TEMPERATURE_AND_HUMIDITY:
      return supla_NewEnum(env, cls_name, "TEMPERATURE_AND_HUMIDITY");
    case SUPLA_DEVCFG_HOME_SCREEN_CONTENT_TIME:
      return supla_NewEnum(env, cls_name, "TIME");
    case SUPLA_DEVCFG_HOME_SCREEN_CONTENT_TIME_DATE:
      return supla_NewEnum(env, cls_name, "TIME_DATE");
    case SUPLA_DEVCFG_HOME_SCREEN_CONTENT_TEMPERATURE_TIME:
      return supla_NewEnum(env, cls_name, "TEMPERATURE_TIME");
    case SUPLA_DEVCFG_HOME_SCREEN_CONTENT_MAIN_AND_AUX_TEMPERATURE:
      return supla_NewEnum(env, cls_name, "MAIN_AND_AUX_TEMPERATURE");
  }

  return env->NewGlobalRef(nullptr);
}

jobject content_available_to_jobject(
    JNIEnv *env, unsigned _supla_int64_t content_available) {
  jobject result = supla_NewEnumSet(
      env, "org/supla/android/data/source/remote/HomeScreenContent");

  for (int a = 0;
       a < sizeof(all_content_types) / sizeof(unsigned _supla_int64_t); a++) {
    if (content_available & all_content_types[a]) {
      jobject jcontent =
          home_screen_content_to_jobject(env, all_content_types[a]);
      if (jcontent) {
        supla_AddItemToEnumSet(env, result, jcontent);
        env->DeleteLocalRef(jcontent);
      }
    }
  }

  return result;
}

jobject home_screen_content_to_jobject(
    JNIEnv *env, jobject jfield_type,
    TDeviceConfig_HomeScreenContent *content) {
  jobject jcontent_available =
      content_available_to_jobject(env, content->ContentAvailable);

  jobject jcontent =
      home_screen_content_to_jobject(env, content->HomeScreenContent);

  jclass cls = env->FindClass(
      "org/supla/android/data/source/remote/HomeScreenContentField");

  jmethodID init_method = env->GetMethodID(
      cls, "<init>",
      "(Lorg/supla/android/data/source/remote/FieldType;Ljava/util/"
      "EnumSet;Lorg/supla/android/data/source/remote/HomeScreenContent;)V");

  return env->NewObject(cls, init_method, jfield_type, jcontent_available,
                        jcontent);
};

jobject available_fields_to_jobject(JNIEnv *env,
                                    unsigned _supla_int64_t available_fields) {
  jobject result =
      supla_NewEnumSet(env, "org/supla/android/data/source/remote/FieldType");

  for (int a = 0;
       a < sizeof(all_device_config_fields) / sizeof(unsigned _supla_int64_t);
       a++) {
    if (available_fields & all_device_config_fields[a]) {
      jobject jfield =
          supla_device_config_field_to_object(env, all_device_config_fields[a]);
      if (jfield) {
        supla_AddItemToEnumSet(env, result, jfield);
        env->DeleteLocalRef(jfield);
      }
    }
  }

  return result;
}

jobject supla_device_config_to_jobject(JNIEnv *env, TSCS_DeviceConfig *config) {
  jobject jfields = supla_NewArrayList(env);
  unsigned short offset = 0;

  unsigned _supla_int64_t max =
      all_device_config_fields[sizeof(all_device_config_fields) /
                                   sizeof(unsigned _supla_int64_t) -
                               1];

  for (int a = 0; a < sizeof(config->Fields) * 8; a++) {
    unsigned _supla_int64_t field = 1 << a;
    if (field > max) {
      break;
    }
    if (config->Fields & field) {
      unsigned _supla_int16_t left = config->ConfigSize;
      if (left > SUPLA_DEVICE_CONFIG_MAXSIZE) {
        left = SUPLA_DEVICE_CONFIG_MAXSIZE;
      }
      if (left > offset) {
        left -= offset;
      } else {
        break;
      }

      void *ptr = &config->Config[offset];
      unsigned short size = 0;

      jobject jfield = env->NewGlobalRef(nullptr);

      jobject jfield_type = supla_device_config_field_to_object(env, field);

      switch (field) {
        case SUPLA_DEVICE_CONFIG_FIELD_STATUS_LED:
          if (left >= (size = sizeof(TDeviceConfig_StatusLed))) {
            jfield = status_led_to_jobject(
                env, jfield_type, static_cast<TDeviceConfig_StatusLed *>(ptr));
          }
          break;
        case SUPLA_DEVICE_CONFIG_FIELD_SCREEN_BRIGHTNESS:
          if (left >= (size = sizeof(TDeviceConfig_ScreenBrightness))) {
            jfield = screen_brightness_to_jobject(
                env, jfield_type,
                static_cast<TDeviceConfig_ScreenBrightness *>(ptr));
          }
          break;
        case SUPLA_DEVICE_CONFIG_FIELD_BUTTON_VOLUME:
          if (left >= (size = sizeof(TDeviceConfig_ButtonVolume))) {
            jfield = button_volume_to_jobject(
                env, jfield_type,
                static_cast<TDeviceConfig_ButtonVolume *>(ptr));
          }
          break;
        case SUPLA_DEVICE_CONFIG_FIELD_DISABLE_USER_INTERFACE:
          if (left >= (size = sizeof(TDeviceConfig_DisableUserInterface))) {
            jfield = disable_user_interface_to_jobject(
                env, jfield_type,
                static_cast<TDeviceConfig_DisableUserInterface *>(ptr));
          }
          break;
        case SUPLA_DEVICE_CONFIG_FIELD_AUTOMATIC_TIME_SYNC:
          if (left >= (size = sizeof(TDeviceConfig_AutomaticTimeSync))) {
            jfield = automatic_time_sync_to_jobject(
                env, jfield_type,
                static_cast<TDeviceConfig_AutomaticTimeSync *>(ptr));
          }
          break;
        case SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_OFF_DELAY:
          if (left >= (size = sizeof(TDeviceConfig_HomeScreenOffDelay))) {
            jfield = home_screen_to_off_delay(
                env, jfield_type,
                static_cast<TDeviceConfig_HomeScreenOffDelay *>(ptr));
          }
          break;
        case SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_CONTENT:
          if (left >= (size = sizeof(TDeviceConfig_HomeScreenContent))) {
            jfield = home_screen_content_to_jobject(
                env, jfield_type,
                static_cast<TDeviceConfig_HomeScreenContent *>(ptr));
          }
          break;
      }

      if (jfield) {
        supla_AddItemToArrayList(env, jfields, jfield);
        env->DeleteLocalRef(jfield);
      }

      env->DeleteLocalRef(jfield_type);

      offset += size;
    }
  }

  jobject javailable_fields =
      available_fields_to_jobject(env, config->AvailableFields);

  jclass config_cls =
      env->FindClass("org/supla/android/data/source/remote/SuplaDeviceConfig");

  jmethodID init_method = env->GetMethodID(
      config_cls, "<init>", "(ILjava/util/EnumSet;Ljava/util/List;)V");

  return env->NewObject(config_cls, init_method, config->DeviceId,
                        javailable_fields, jfields);
}

void supla_cb_on_device_config_update_or_result(
    void *_suplaclient, void *user_data,
    TSC_DeviceConfigUpdateOrResult *result) {
  ASC_VAR_DECLARATION();
  ENV_VAR_DECLARATION();

  jobject jresult = supla_config_result_to_jobject(env, result->Result);
  jobject jdevice_config = supla_device_config_to_jobject(env, &result->Config);
  jboolean eol = result->Config.EndOfDataFlag ? JNI_TRUE : JNI_FALSE;

  env->CallVoidMethod(asc->j_obj, asc->j_mid_on_device_config_update_or_result,
                      jdevice_config, jresult, eol);

  env->DeleteLocalRef(jresult);
  env->DeleteLocalRef(jdevice_config);
}

void supla_device_config_init(JNIEnv *env, jclass oclass,
                              TAndroidSuplaClient *asc,
                              TSuplaClientCfg *sclient_cfg) {
  asc->j_mid_on_device_config_update_or_result = env->GetMethodID(
      oclass, "onDeviceConfigUpdateOrResult",
      "(Lorg/supla/android/data/source/remote/SuplaDeviceConfig;Lorg/supla/"
      "android/data/source/remote/ConfigResult;Z)V");

  sclient_cfg->cb_on_device_config_update_or_result =
      supla_cb_on_device_config_update_or_result;
}

unsigned _supla_int64_t supla_device_config_enumset_to_bits(JNIEnv *env,
                                                            jobject fields) {
  unsigned _supla_int64_t result = 0;

  jclass enum_set_cls = env->FindClass("java/util/EnumSet");

  jmethodID containts_method =
      env->GetMethodID(enum_set_cls, "contains", "(Ljava/lang/Object;)Z");

  for (int a = 0;
       a < sizeof(all_device_config_fields) / sizeof(unsigned _supla_int64_t);
       a++) {
    jobject jfield =
        supla_device_config_field_to_object(env, all_device_config_fields[a]);
    if (jfield) {
      if (env->CallBooleanMethod(fields, containts_method, jfield)) {
        result |= all_device_config_fields[a];
      }
      env->DeleteLocalRef(jfield);
    }
  }

  env->DeleteLocalRef(enum_set_cls);
  return result;
}

JNIEXPORT jboolean JNICALL
Java_org_supla_android_lib_SuplaClient_scGetDeviceConfig(
    JNIEnv *env, jobject thiz, jlong _asc, jint device_id, jobject fields) {
  jboolean result = JNI_FALSE;
  void *supla_client = supla_client_ptr(_asc);
  if (supla_client) {
    TCS_GetDeviceConfigRequest request = {};
    request.DeviceId = device_id;
    request.Fields = supla_device_config_enumset_to_bits(env, fields);
    if (!request.Fields) {
      return JNI_FALSE;
    }

    result = supla_client_get_device_config(supla_client, &request) > 0
                 ? JNI_TRUE
                 : JNI_FALSE;
  }

  return result;
}
