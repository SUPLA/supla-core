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

#include "mqtt_abstract_state_message_provider.h"

#include <stdlib.h>
#include <string.h>

#include "device/channel_fragment.h"
#include "device/extended_value/channel_ic_extended_value.h"
#include "device/value/channel_binary_sensor_value.h"
#include "device/value/channel_floating_point_sensor_value.h"
#include "device/value/channel_gate_value.h"
#include "device/value/channel_onoff_value.h"
#include "device/value/channel_openclosed_value.h"
#include "device/value/channel_rgbw_value.h"
#include "device/value/channel_rs_value.h"
#include "device/value/channel_temphum_value.h"
#include "device/value/channel_thermostat_value.h"
#include "device/value/channel_valve_value.h"
#include "log.h"

supla_mqtt_abstract_state_message_provider::
    supla_mqtt_abstract_state_message_provider(void)
    : supla_mqtt_message_provider() {
  this->user_id = 0;
  this->device_id = 0;
  this->channel_id = 0;
  this->channel_online = false;
  this->channel_function = 0;
  this->channel_flags = 0;
  this->channel_value = nullptr;
  this->channel_extended_value = nullptr;
  this->user_suid = nullptr;
}

supla_mqtt_abstract_state_message_provider::
    ~supla_mqtt_abstract_state_message_provider(void) {
  if (channel_value) {
    delete channel_value;
  }

  if (channel_extended_value) {
    delete channel_extended_value;
  }
}

int supla_mqtt_abstract_state_message_provider::get_user_id(void) {
  return user_id;
}

int supla_mqtt_abstract_state_message_provider::get_device_id(void) {
  return device_id;
}

int supla_mqtt_abstract_state_message_provider::get_channel_id(void) {
  return channel_id;
}

void supla_mqtt_abstract_state_message_provider::set_ids(int user_id,
                                                         int device_id,
                                                         int channel_id) {
  this->user_id = user_id;
  this->device_id = device_id;
  this->channel_id = channel_id;

  if (channel_value) {
    delete channel_value;
    channel_value = nullptr;
  }

  if (channel_extended_value) {
    delete channel_extended_value;
    channel_extended_value = nullptr;
  }
}

void supla_mqtt_abstract_state_message_provider::set_channel_function_and_flags(
    int channel_function, int channel_flags) {
  this->channel_function = channel_function;

  this->channel_flags = channel_flags;
}

void supla_mqtt_abstract_state_message_provider::set_user_suid(
    char *user_suid) {
  this->user_suid = user_suid;
}

void supla_mqtt_abstract_state_message_provider::set_user_suid(void) {
  user_suid = (char *)_get_user_suid();
}

void supla_mqtt_abstract_state_message_provider::get_temperature(
    char *buf, size_t buf_size) {
  supla_channel_temphum_value *temphum_val =
      dynamic_cast<supla_channel_temphum_value *>(channel_value);
  if (temphum_val) {
    snprintf(buf, buf_size, "%f", temphum_val->get_temperature());
  }
}

void supla_mqtt_abstract_state_message_provider::get_humidity(char *buf,
                                                              size_t buf_size) {
  supla_channel_temphum_value *temphum_val =
      dynamic_cast<supla_channel_temphum_value *>(channel_value);
  if (temphum_val) {
    snprintf(buf, buf_size, "%f", temphum_val->get_humidity());
  }
}

void supla_mqtt_abstract_state_message_provider::get_brightness(
    char brightness, char *buf, size_t buf_size) {
  snprintf(buf, buf_size, "%i", brightness);
}

void supla_mqtt_abstract_state_message_provider::get_color(char *buf,
                                                           size_t buf_size) {
  supla_channel_rgbw_value *rgbw_val =
      dynamic_cast<supla_channel_rgbw_value *>(channel_value);

  if (rgbw_val) {
    int color = rgbw_val->get_color();
    snprintf(buf, buf_size, "0x%02X%02X%02X", ((unsigned char *)&color)[2],
             ((unsigned char *)&color)[1], ((unsigned char *)&color)[0]);
  }
}

void supla_mqtt_abstract_state_message_provider::get_rgb(char *buf,
                                                         size_t buf_size) {
  supla_channel_rgbw_value *rgbw_val =
      dynamic_cast<supla_channel_rgbw_value *>(channel_value);

  if (rgbw_val) {
    int color = rgbw_val->get_color();
    snprintf(buf, buf_size, "%i,%i,%i", ((unsigned char *)&color)[2],
             ((unsigned char *)&color)[1], ((unsigned char *)&color)[0]);
  }
}

void supla_mqtt_abstract_state_message_provider::get_valve_closed(
    char *buf, size_t buf_size) {
  supla_channel_valve_value *valve_val =
      dynamic_cast<supla_channel_valve_value *>(channel_value);

  if (valve_val) {
    if (channel_function == SUPLA_CHANNELFNC_VALVE_PERCENTAGE) {
      snprintf(buf, buf_size, "%i",
               valve_val->get_valve_value()->closed_percent);
    } else {
      snprintf(buf, buf_size, "%s",
               valve_val->get_valve_value()->closed ? "true" : "false");
    }
  }
}

bool supla_mqtt_abstract_state_message_provider::get_rs_message_at_index(
    unsigned short index, const char *topic_prefix, char **topic_name,
    void **message, size_t *message_size) {
  supla_channel_rs_value *rs_val = nullptr;

  if (index >= 1 && index <= 3) {
    rs_val = dynamic_cast<supla_channel_rs_value *>(channel_value);
  }

  char value[50];
  value[0] = 0;

  switch (index) {
    case 1:
      return create_message(
          topic_prefix, user_suid, topic_name, message, message_size,
          rs_val && rs_val->get_opening_sensor_level() == rsl_closed ? "true"
                                                                     : "false",
          false, "devices/%i/channels/%i/state/hi", get_device_id(),
          get_channel_id());
    case 2:
      return create_message(
          topic_prefix, user_suid, topic_name, message, message_size,
          rs_val && rs_val->get_rs_value()->position == -1 ? "true" : "false",
          false, "devices/%i/channels/%i/state/is_calibrating", get_device_id(),
          get_channel_id());
    case 3: {
      short shut = rs_val ? rs_val->get_rs_value()->position : 0;
      if (shut < 0) {
        shut = 0;
      } else if (shut > 100) {
        shut = 100;
      }
      snprintf(value, sizeof(value), "%i", shut);
      return create_message(topic_prefix, user_suid, topic_name, message,
                            message_size, value, false,
                            "devices/%i/channels/%i/state/shut",
                            get_device_id(), get_channel_id());
    }
  }

  return false;
}

bool supla_mqtt_abstract_state_message_provider::get_lck_message_at_index(
    unsigned short index, const char *topic_prefix, char **topic_name,
    void **message, size_t *message_size) {
  if (index == 1) {
    supla_channel_openclosed_value *oc_val =
        dynamic_cast<supla_channel_openclosed_value *>(channel_value);

    return create_message(
        topic_prefix, user_suid, topic_name, message, message_size,
        oc_val && oc_val->is_closed() ? "true" : "false", false,
        "devices/%i/channels/%i/state/hi", get_device_id(), get_channel_id());
  }

  return false;
}

bool supla_mqtt_abstract_state_message_provider::get_gate_message_at_index(
    unsigned short index, const char *topic_prefix, char **topic_name,
    void **message, size_t *message_size) {
  supla_channel_gate_value *gate_val = nullptr;

  if (index >= 1 && index <= 2) {
    gate_val = dynamic_cast<supla_channel_gate_value *>(channel_value);
  }

  switch (index) {
    case 1:
      return create_message(
          topic_prefix, user_suid, topic_name, message, message_size,
          gate_val && gate_val->get_opening_sensor_level() == gsl_closed
              ? "true"
              : "false",
          false, "devices/%i/channels/%i/state/hi", get_device_id(),
          get_channel_id());
    case 2:
      return create_message(
          topic_prefix, user_suid, topic_name, message, message_size,
          gate_val && gate_val->get_partial_opening_sensor_level() == gsl_closed
              ? "true"
              : "false",
          false, "devices/%i/channels/%i/state/partial_hi", get_device_id(),
          get_channel_id());
  }

  return false;
}

bool supla_mqtt_abstract_state_message_provider::get_onoff_message_at_index(
    bool on, unsigned short index, const char *topic_prefix, char **topic_name,
    void **message, size_t *message_size, bool *overcurrent_relay_off) {
  if (index == 1) {
    return create_message(topic_prefix, user_suid, topic_name, message,
                          message_size, on ? "true" : "false", false,
                          "devices/%i/channels/%i/state/on", get_device_id(),
                          get_channel_id());
  } else if (index == 2 && overcurrent_relay_off) {
    return create_message(topic_prefix, user_suid, topic_name, message,
                          message_size,
                          *overcurrent_relay_off ? "true" : "false", false,
                          "devices/%i/channels/%i/state/overcurrent_relay_off",
                          get_device_id(), get_channel_id());
  }

  return false;
}

bool supla_mqtt_abstract_state_message_provider::get_onoff_message_at_index(
    bool on, unsigned short index, const char *topic_prefix, char **topic_name,
    void **message, size_t *message_size) {
  return get_onoff_message_at_index(on, index, topic_prefix, topic_name,
                                    message, message_size, nullptr);
}

bool supla_mqtt_abstract_state_message_provider::get_depth_message_at_index(
    unsigned short index, const char *topic_prefix, char **topic_name,
    void **message, size_t *message_size) {
  if (index == 1) {
    char value[50];
    value[0] = 0;

    supla_channel_floating_point_sensor_value *f_val =
        dynamic_cast<supla_channel_floating_point_sensor_value *>(
            channel_value);

    if (f_val) {
      snprintf(value, sizeof(value), "%f", f_val->get_value());
    }
    return create_message(topic_prefix, user_suid, topic_name, message,
                          message_size, value, false,
                          "devices/%i/channels/%i/state/depth", get_device_id(),
                          get_channel_id());
  }

  return false;
}

bool supla_mqtt_abstract_state_message_provider::get_distance_message_at_index(
    unsigned short index, const char *topic_prefix, char **topic_name,
    void **message, size_t *message_size) {
  if (index == 1) {
    char value[50];
    value[0] = 0;

    supla_channel_floating_point_sensor_value *f_val =
        dynamic_cast<supla_channel_floating_point_sensor_value *>(
            channel_value);

    if (f_val) {
      snprintf(value, sizeof(value), "%f", f_val->get_value());
    }
    return create_message(topic_prefix, user_suid, topic_name, message,
                          message_size, value, false,
                          "devices/%i/channels/%i/state/distance",
                          get_device_id(), get_channel_id());
  }

  return false;
}

bool supla_mqtt_abstract_state_message_provider::
    get_doublevalue_message_at_index(unsigned short index,
                                     const char *topic_prefix,
                                     char **topic_name, void **message,
                                     size_t *message_size) {
  if (index == 1) {
    char value[50];
    value[0] = 0;

    supla_channel_floating_point_sensor_value *f_val =
        dynamic_cast<supla_channel_floating_point_sensor_value *>(
            channel_value);

    if (f_val) {
      switch (channel_function) {
        case SUPLA_CHANNELFNC_PRESSURESENSOR:
        case SUPLA_CHANNELFNC_RAINSENSOR:
        case SUPLA_CHANNELFNC_WEIGHTSENSOR:
        case SUPLA_CHANNELFNC_WINDSENSOR:
          snprintf(value, sizeof(value), "%f", f_val->get_value());
          break;
      }
    }

    return create_message(topic_prefix, user_suid, topic_name, message,
                          message_size, value, false,
                          "devices/%i/channels/%i/state/value", get_device_id(),
                          get_channel_id());
  }

  return false;
}

bool supla_mqtt_abstract_state_message_provider::get_sensor_message_at_index(
    unsigned short index, const char *topic_prefix, char **topic_name,
    void **message, size_t *message_size) {
  if (index == 1) {
    supla_channel_binary_sensor_value *bin_val =
        dynamic_cast<supla_channel_binary_sensor_value *>(channel_value);

    return create_message(
        topic_prefix, user_suid, topic_name, message, message_size,
        bin_val && bin_val->is_hi() ? "true" : "false", false,
        "devices/%i/channels/%i/state/hi", get_device_id(), get_channel_id());
  }

  return false;
}

bool supla_mqtt_abstract_state_message_provider::
    get_temperature_message_at_index(unsigned short index,
                                     unsigned short expected_index,
                                     const char *topic_prefix,
                                     char **topic_name, void **message,
                                     size_t *message_size) {
  if (index == expected_index) {
    char value[50];
    value[0] = 0;

    get_temperature(value, sizeof(value));
    return create_message(topic_prefix, user_suid, topic_name, message,
                          message_size, value, false,
                          "devices/%i/channels/%i/state/temperature",
                          get_device_id(), get_channel_id());
  }

  return false;
}

bool supla_mqtt_abstract_state_message_provider::get_humidity_message_at_index(
    unsigned short index, unsigned short expected_index,
    const char *topic_prefix, char **topic_name, void **message,
    size_t *message_size) {
  if (index == expected_index) {
    char value[50];
    value[0] = 0;

    get_humidity(value, sizeof(value));
    return create_message(topic_prefix, user_suid, topic_name, message,
                          message_size, value, false,
                          "devices/%i/channels/%i/state/humidity",
                          get_device_id(), get_channel_id());
  }

  return false;
}

bool supla_mqtt_abstract_state_message_provider::get_brightness_message(
    const char *topic_prefix, char **topic_name, void **message,
    size_t *message_size) {
  char value[50];
  value[0] = 0;

  supla_channel_rgbw_value *rgbw_val =
      dynamic_cast<supla_channel_rgbw_value *>(channel_value);

  if (rgbw_val) {
    get_brightness(rgbw_val->get_brightness(), value, sizeof(value));
  }

  return create_message(topic_prefix, user_suid, topic_name, message,
                        message_size, value, false,
                        "devices/%i/channels/%i/state/brightness",
                        get_device_id(), get_channel_id());
}

bool supla_mqtt_abstract_state_message_provider::get_color_brightness_message(
    const char *topic_prefix, char **topic_name, void **message,
    size_t *message_size) {
  char value[50];
  value[0] = 0;

  supla_channel_rgbw_value *rgbw_val =
      dynamic_cast<supla_channel_rgbw_value *>(channel_value);

  if (rgbw_val) {
    get_brightness(rgbw_val->get_color_brightness(), value, sizeof(value));
  }

  return create_message(topic_prefix, user_suid, topic_name, message,
                        message_size, value, false,
                        "devices/%i/channels/%i/state/color_brightness",
                        get_device_id(), get_channel_id());
}

bool supla_mqtt_abstract_state_message_provider::get_color_message(
    const char *topic_prefix, char **topic_name, void **message,
    size_t *message_size) {
  char value[50];
  value[0] = 0;

  get_color(value, sizeof(value));
  return create_message(
      topic_prefix, user_suid, topic_name, message, message_size, value, false,
      "devices/%i/channels/%i/state/color", get_device_id(), get_channel_id());
}

bool supla_mqtt_abstract_state_message_provider::get_normalized_rgb_message(
    const char *topic_prefix, char **topic_name, void **message,
    size_t *message_size) {
  char value[50];
  value[0] = 0;

  get_rgb(value, sizeof(value));
  return create_message(topic_prefix, user_suid, topic_name, message,
                        message_size, value, false,
                        "devices/%i/channels/%i/state/normalized_rgb",
                        get_device_id(), get_channel_id());
}

bool supla_mqtt_abstract_state_message_provider::get_valve_message_at_index(
    unsigned short index, const char *topic_prefix, char **topic_name,
    void **message, size_t *message_size) {
  char value[50];
  value[0] = 0;

  supla_channel_valve_value *valve_val = nullptr;

  if (index >= 1 && index <= 3) {
    valve_val = dynamic_cast<supla_channel_valve_value *>(channel_value);
  }

  switch (index) {
    case 1:
      get_valve_closed(value, sizeof(value));
      return create_message(topic_prefix, user_suid, topic_name, message,
                            message_size, value, false,
                            "devices/%i/channels/%i/state/closed",
                            get_device_id(), get_channel_id());
    case 2:
      return create_message(
          topic_prefix, user_suid, topic_name, message, message_size,
          valve_val && (valve_val->get_valve_value()->flags &
                        SUPLA_VALVE_FLAG_MANUALLY_CLOSED)
              ? "true"
              : "false",
          false, "devices/%i/channels/%i/state/manually_closed",
          get_device_id(), get_channel_id());

    case 3:
      return create_message(topic_prefix, user_suid, topic_name, message,
                            message_size,
                            valve_val && (valve_val->get_valve_value()->flags &
                                          SUPLA_VALVE_FLAG_FLOODING)
                                ? "true"
                                : "false",
                            false, "devices/%i/channels/%i/state/flooding",
                            get_device_id(), get_channel_id());
  }

  return false;
}

bool supla_mqtt_abstract_state_message_provider::
    get_impulsecounter_message_at_index(unsigned short index,
                                        const char *topic_prefix,
                                        char **topic_name, void **message,
                                        size_t *message_size) {
  if (channel_extended_value == nullptr) {
    channel_extended_value = _get_channel_property_getter()->get_extended_value(
        get_user_id(), get_device_id(), get_channel_id());
  }

  supla_channel_ic_extended_value *icv =
      dynamic_cast<supla_channel_ic_extended_value *>(channel_extended_value);

  if (icv == nullptr) {
    message = nullptr;

    if (message_size) {
      *message_size = 0;
      message_size = nullptr;
    }
  }

  char value[50];
  value[0] = 0;

  switch (index) {
    case 1:
      snprintf(value, sizeof(value), "%.2f",
               icv ? (icv->get_total_cost() * 0.01) : 0);
      return create_message(topic_prefix, user_suid, topic_name, message,
                            message_size, value, false,
                            "devices/%i/channels/%i/state/total_cost",
                            get_device_id(), get_channel_id());
    case 2:
      snprintf(value, sizeof(value), "%.4f",
               icv ? (icv->get_price_per_unit() * 0.0001) : 0);
      return create_message(topic_prefix, user_suid, topic_name, message,
                            message_size, value, false,
                            "devices/%i/channels/%i/state/price_per_unit",
                            get_device_id(), get_channel_id());
    case 3:
      snprintf(value, sizeof(value), "%i",
               icv ? icv->get_impulses_per_unit() : 0);
      return create_message(topic_prefix, user_suid, topic_name, message,
                            message_size, value, false,
                            "devices/%i/channels/%i/state/impulses_per_unit",
                            get_device_id(), get_channel_id());
    case 4:
      snprintf(value, sizeof(value), "%llu", icv ? icv->get_counter() : 0);
      return create_message(topic_prefix, user_suid, topic_name, message,
                            message_size, value, false,
                            "devices/%i/channels/%i/state/counter",
                            get_device_id(), get_channel_id());
    case 5:
      snprintf(value, sizeof(value), "%.3f",
               icv ? (icv->get_calculated_value() * 0.001) : 0);
      return create_message(topic_prefix, user_suid, topic_name, message,
                            message_size, value, false,
                            "devices/%i/channels/%i/state/calculated_value",
                            get_device_id(), get_channel_id());
    case 6:
      return create_message(topic_prefix, user_suid, topic_name, message,
                            message_size,
                            icv ? icv->get_currency().c_str() : "", false,
                            "devices/%i/channels/%i/state/currency",
                            get_device_id(), get_channel_id());
    case 7:
      return create_message(topic_prefix, user_suid, topic_name, message,
                            message_size,
                            icv ? icv->get_custom_unit().c_str() : "", false,
                            "devices/%i/channels/%i/state/unit",
                            get_device_id(), get_channel_id());
  }

  return false;
}

void supla_mqtt_abstract_state_message_provider::verify_flag(
    supla_channel_em_extended_value **em, int flags, int flag1, int flag2,
    void **message, size_t *message_size) {
  if ((flags & flag1) == 0 && (flags & flag2) == 0) {
    *em = nullptr;
    if (message) {
      *message = nullptr;
    }
    if (message_size) {
      *message_size = 0;
    }
  }
}

bool supla_mqtt_abstract_state_message_provider::
    get_electricitymeter_message_at_index(unsigned short index,
                                          const char *topic_prefix,
                                          char **topic_name, void **message,
                                          size_t *message_size) {
  if (index > 45) {
    return false;
  }

  if (channel_extended_value == nullptr) {
    channel_extended_value = _get_channel_property_getter()->get_extended_value(
        get_user_id(), get_device_id(), get_channel_id());
  }

  supla_channel_em_extended_value *emv =
      dynamic_cast<supla_channel_em_extended_value *>(channel_extended_value);

  TElectricityMeter_ExtendedValue_V2 em_ev = {};

  if (emv) {
    emv->get_raw_value(&em_ev);
    if (em_ev.m_count == 0) {
      emv = nullptr;
    }
  }

  if (((channel_flags & SUPLA_CHANNEL_FLAG_PHASE1_UNSUPPORTED) && index >= 10 &&
       index <= 21) ||
      (((channel_flags & SUPLA_CHANNEL_FLAG_PHASE2_UNSUPPORTED) &&
        index >= 22 && index <= 33)) ||
      (((channel_flags & SUPLA_CHANNEL_FLAG_PHASE3_UNSUPPORTED) &&
        index >= 34 && index <= 45))) {
    emv = nullptr;
  }

  if (emv == nullptr) {
    message = nullptr;

    if (message_size) {
      *message_size = 0;
      message_size = nullptr;
    }
  }

  char value[50] = {};

  short phase = (index - 10) / 12;

  switch (index) {
    case 1:
      snprintf(value, sizeof(value), "%.2f",
               emv ? (em_ev.total_cost * 0.01) : 0);
      return create_message(topic_prefix, user_suid, topic_name, message,
                            message_size, value, false,
                            "devices/%i/channels/%i/state/total_cost",
                            get_device_id(), get_channel_id());
    case 2:
      snprintf(value, sizeof(value), "%.2f",
               emv ? (em_ev.total_cost_balanced * 0.01) : 0);
      return create_message(topic_prefix, user_suid, topic_name, message,
                            message_size, value, false,
                            "devices/%i/channels/%i/state/total_cost_balanced",
                            get_device_id(), get_channel_id());

    case 3:
      snprintf(value, sizeof(value), "%.4f",
               emv ? (em_ev.price_per_unit * 0.0001) : 0);
      return create_message(topic_prefix, user_suid, topic_name, message,
                            message_size, value, false,
                            "devices/%i/channels/%i/state/price_per_unit",
                            get_device_id(), get_channel_id());

    case 4:
      return create_message(topic_prefix, user_suid, topic_name, message,
                            message_size,
                            emv ? emv->get_currency().c_str() : "", false,
                            "devices/%i/channels/%i/state/currency",
                            get_device_id(), get_channel_id());

    case 5:
      snprintf(value, sizeof(value), "%u", emv ? em_ev.measured_values : 0);
      // This topic should be called "measured_values" but for compatibility
      // with the rest API it has been changed to "support"
      return create_message(topic_prefix, user_suid, topic_name, message,
                            message_size, value, false,
                            "devices/%i/channels/%i/state/support",
                            get_device_id(), get_channel_id());

    case 6:
      verify_flag(&emv, em_ev.measured_values, EM_VAR_FORWARD_ACTIVE_ENERGY, 0,
                  message, message_size);
      snprintf(value, sizeof(value), "%.5f",
               emv ? ((em_ev.total_forward_active_energy[0] +
                       em_ev.total_forward_active_energy[1] +
                       em_ev.total_forward_active_energy[2]) *
                      0.00001)
                   : 0);
      return create_message(
          topic_prefix, user_suid, topic_name, message, message_size, value,
          false, "devices/%i/channels/%i/state/total_forward_active_energy",
          get_device_id(), get_channel_id());

    case 7:
      verify_flag(&emv, em_ev.measured_values, EM_VAR_REVERSE_ACTIVE_ENERGY, 0,
                  message, message_size);
      snprintf(value, sizeof(value), "%.5f",
               emv ? ((em_ev.total_reverse_active_energy[0] +
                       em_ev.total_reverse_active_energy[1] +
                       em_ev.total_reverse_active_energy[2]) *
                      0.00001)
                   : 0);
      return create_message(
          topic_prefix, user_suid, topic_name, message, message_size, value,
          false, "devices/%i/channels/%i/state/total_reverse_active_energy",
          get_device_id(), get_channel_id());

    case 8:
      verify_flag(&emv, em_ev.measured_values,
                  EM_VAR_FORWARD_ACTIVE_ENERGY_BALANCED, 0, message,
                  message_size);
      snprintf(
          value, sizeof(value), "%.5f",
          emv ? (em_ev.total_forward_active_energy_balanced * 0.00001) : 0);
      return create_message(
          topic_prefix, user_suid, topic_name, message, message_size, value,
          false,
          "devices/%i/channels/%i/state/total_forward_active_energy_balanced",
          get_device_id(), get_channel_id());

    case 9:
      verify_flag(&emv, em_ev.measured_values,
                  EM_VAR_REVERSE_ACTIVE_ENERGY_BALANCED, 0, message,
                  message_size);
      snprintf(
          value, sizeof(value), "%.5f",
          emv ? (em_ev.total_reverse_active_energy_balanced * 0.00001) : 0);
      return create_message(
          topic_prefix, user_suid, topic_name, message, message_size, value,
          false,
          "devices/%i/channels/%i/state/total_reverse_active_energy_balanced",
          get_device_id(), get_channel_id());

    case 10:
    case 22:
    case 34:
      verify_flag(&emv, em_ev.measured_values, EM_VAR_FORWARD_ACTIVE_ENERGY, 0,
                  message, message_size);
      snprintf(value, sizeof(value), "%.5f",
               emv ? (em_ev.total_forward_active_energy[phase] * 0.00001) : 0);
      return create_message(
          topic_prefix, user_suid, topic_name, message, message_size, value,
          false,
          "devices/%i/channels/%i/state/phases/%i/total_forward_active_energy",
          get_device_id(), get_channel_id(), phase + 1);

    case 11:
    case 23:
    case 35:
      verify_flag(&emv, em_ev.measured_values, EM_VAR_REVERSE_ACTIVE_ENERGY, 0,
                  message, message_size);
      snprintf(value, sizeof(value), "%.5f",
               emv ? (em_ev.total_reverse_active_energy[phase] * 0.00001) : 0);
      return create_message(
          topic_prefix, user_suid, topic_name, message, message_size, value,
          false,
          "devices/%i/channels/%i/state/phases/%i/total_reverse_active_energy",
          get_device_id(), get_channel_id(), phase + 1);

    case 12:
    case 24:
    case 36:
      verify_flag(&emv, em_ev.measured_values, EM_VAR_FORWARD_REACTIVE_ENERGY,
                  0, message, message_size);
      snprintf(
          value, sizeof(value), "%.5f",
          emv ? (em_ev.total_forward_reactive_energy[phase] * 0.00001) : 0);
      return create_message(topic_prefix, user_suid, topic_name, message,
                            message_size, value, false,
                            "devices/%i/channels/%i/state/phases/%i/"
                            "total_forward_reactive_energy",
                            get_device_id(), get_channel_id(), phase + 1);

    case 13:
    case 25:
    case 37:
      verify_flag(&emv, em_ev.measured_values, EM_VAR_REVERSE_REACTIVE_ENERGY,
                  0, message, message_size);
      snprintf(
          value, sizeof(value), "%.5f",
          emv ? (em_ev.total_reverse_reactive_energy[phase] * 0.00001) : 0);
      return create_message(topic_prefix, user_suid, topic_name, message,
                            message_size, value, false,
                            "devices/%i/channels/%i/state/phases/%i/"
                            "total_reverse_reactive_energy",
                            get_device_id(), get_channel_id(), phase + 1);

    case 14:
    case 26:
    case 38:
      verify_flag(&emv, em_ev.measured_values, EM_VAR_FREQ, 0, message,
                  message_size);
      snprintf(value, sizeof(value), "%.2f",
               emv ? (em_ev.m[0].freq * 0.01) : 0);
      return create_message(topic_prefix, user_suid, topic_name, message,
                            message_size, value, false,
                            "devices/%i/channels/%i/state/phases/%i/frequency",
                            get_device_id(), get_channel_id(), phase + 1);

    case 15:
    case 27:
    case 39:
      verify_flag(&emv, em_ev.measured_values, EM_VAR_VOLTAGE, 0, message,
                  message_size);
      snprintf(value, sizeof(value), "%.2f",
               emv ? (em_ev.m[0].voltage[phase] * 0.01) : 0);
      return create_message(topic_prefix, user_suid, topic_name, message,
                            message_size, value, false,
                            "devices/%i/channels/%i/state/phases/%i/voltage",
                            get_device_id(), get_channel_id(), phase + 1);

    case 16:
    case 28:
    case 40: {
      verify_flag(&emv, em_ev.measured_values, EM_VAR_CURRENT,
                  EM_VAR_CURRENT_OVER_65A, message, message_size);
      unsigned int current = em_ev.m[0].current[phase];

      if ((em_ev.measured_values & EM_VAR_CURRENT_OVER_65A) &&
          !(em_ev.measured_values & EM_VAR_CURRENT)) {
        current *= 10;
      }

      snprintf(value, sizeof(value), "%.3f", emv ? (current * 0.001) : 0);
      return create_message(topic_prefix, user_suid, topic_name, message,
                            message_size, value, false,
                            "devices/%i/channels/%i/state/phases/%i/current",
                            get_device_id(), get_channel_id(), phase + 1);
    }
    case 17:
    case 29:
    case 41:
      verify_flag(&emv, em_ev.measured_values, EM_VAR_POWER_ACTIVE, 0, message,
                  message_size);
      {
        double power = emv ? (em_ev.m[0].power_active[phase] * 0.00001) : 0;
        if (em_ev.measured_values & EM_VAR_POWER_ACTIVE_KW) {
          power *= 1000;
        }
        snprintf(value, sizeof(value), "%.5f", power);
      }
      return create_message(
          topic_prefix, user_suid, topic_name, message, message_size, value,
          false, "devices/%i/channels/%i/state/phases/%i/power_active",
          get_device_id(), get_channel_id(), phase + 1);
    case 18:
    case 30:
    case 42:
      verify_flag(&emv, em_ev.measured_values, EM_VAR_POWER_REACTIVE, 0,
                  message, message_size);
      {
        double power = emv ? (em_ev.m[0].power_reactive[phase] * 0.00001) : 0;
        if (em_ev.measured_values & EM_VAR_POWER_REACTIVE_KVAR) {
          power *= 1000;
        }
        snprintf(value, sizeof(value), "%.5f", power);
      }
      return create_message(
          topic_prefix, user_suid, topic_name, message, message_size, value,
          false, "devices/%i/channels/%i/state/phases/%i/power_reactive",
          get_device_id(), get_channel_id(), phase + 1);
    case 19:
    case 31:
    case 43:
      verify_flag(&emv, em_ev.measured_values, EM_VAR_POWER_APPARENT, 0,
                  message, message_size);
      {
        double power = emv ? (em_ev.m[0].power_apparent[phase] * 0.00001) : 0;
        if (em_ev.measured_values & EM_VAR_POWER_APPARENT_KVA) {
          power *= 1000;
        }
        snprintf(value, sizeof(value), "%.5f", power);
      }
      return create_message(
          topic_prefix, user_suid, topic_name, message, message_size, value,
          false, "devices/%i/channels/%i/state/phases/%i/power_apparent",
          get_device_id(), get_channel_id(), phase + 1);
    case 20:
    case 32:
    case 44:
      verify_flag(&emv, em_ev.measured_values, EM_VAR_POWER_FACTOR, 0, message,
                  message_size);
      snprintf(value, sizeof(value), "%.3f",
               emv ? (em_ev.m[0].power_factor[phase] * 0.001) : 0);
      return create_message(
          topic_prefix, user_suid, topic_name, message, message_size, value,
          false, "devices/%i/channels/%i/state/phases/%i/power_factor",
          get_device_id(), get_channel_id(), phase + 1);
    case 21:
    case 33:
    case 45:
      verify_flag(&emv, em_ev.measured_values, EM_VAR_PHASE_ANGLE, 0, message,
                  message_size);
      snprintf(value, sizeof(value), "%.1f",
               emv ? (em_ev.m[0].phase_angle[phase] * 0.1) : 0);
      return create_message(
          topic_prefix, user_suid, topic_name, message, message_size, value,
          false, "devices/%i/channels/%i/state/phases/%i/phase_angle",
          get_device_id(), get_channel_id(), phase + 1);
  }

  return false;
}

bool supla_mqtt_abstract_state_message_provider::get_message_at_index(
    unsigned short index, const char *topic_prefix, char **topic_name,
    void **message, size_t *message_size) {
  if (user_id == 0 && device_id == 0 && channel_id == 0) {
    return false;
  }

  if (channel_value == nullptr) {
    supla_channel_fragment fragment;
    channel_online = false;
    channel_value = _get_channel_property_getter()->get_value(
        user_id, device_id, channel_id, &fragment, &channel_online);

    if (fragment.get_channel_id()) {
      channel_function = fragment.get_function();
      channel_flags = fragment.get_flags();
    }
  }

  if (index == 0) {
    return create_message(topic_prefix, user_suid, topic_name, message,
                          message_size, channel_online ? "true" : "false",
                          false, "devices/%i/channels/%i/state/connected",
                          get_device_id(), get_channel_id());
  }

  if (!channel_value || !channel_online) {
    message = nullptr;

    if (message_size) {
      *message_size = 0;
      message_size = nullptr;
    }
  }

  switch (channel_function) {
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROOFWINDOW:
      return get_rs_message_at_index(index, topic_prefix, topic_name, message,
                                     message_size);

    case SUPLA_CHANNELFNC_CONTROLLINGTHEGATE:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR:
      return get_gate_message_at_index(index, topic_prefix, topic_name, message,
                                       message_size);

    case SUPLA_CHANNELFNC_CONTROLLINGTHEDOORLOCK:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGATEWAYLOCK:
      return get_lck_message_at_index(index, topic_prefix, topic_name, message,
                                      message_size);

    case SUPLA_CHANNELFNC_THERMOSTAT:
    case SUPLA_CHANNELFNC_THERMOSTAT_HEATPOL_HOMEPLUS: {
      supla_channel_thermostat_value *thermo_val =
          dynamic_cast<supla_channel_thermostat_value *>(channel_value);

      return get_onoff_message_at_index(thermo_val && thermo_val->is_on(),
                                        index, topic_prefix, topic_name,
                                        message, message_size);
    }

    case SUPLA_CHANNELFNC_POWERSWITCH:
    case SUPLA_CHANNELFNC_LIGHTSWITCH:
    case SUPLA_CHANNELFNC_STAIRCASETIMER: {
      supla_channel_onoff_value *onoff_val =
          dynamic_cast<supla_channel_onoff_value *>(channel_value);

      bool overcurrent_relay_off =
          onoff_val && onoff_val->is_overcurrent_relay_off();
      return get_onoff_message_at_index(onoff_val && onoff_val->is_on(), index,
                                        topic_prefix, topic_name, message,
                                        message_size, &overcurrent_relay_off);
    }

    case SUPLA_CHANNELFNC_DEPTHSENSOR:
      return get_depth_message_at_index(index, topic_prefix, topic_name,
                                        message, message_size);

    case SUPLA_CHANNELFNC_DISTANCESENSOR:
      return get_distance_message_at_index(index, topic_prefix, topic_name,
                                           message, message_size);

    case SUPLA_CHANNELFNC_PRESSURESENSOR:
    case SUPLA_CHANNELFNC_RAINSENSOR:
    case SUPLA_CHANNELFNC_WEIGHTSENSOR:
    case SUPLA_CHANNELFNC_WINDSENSOR:
      return get_doublevalue_message_at_index(index, topic_prefix, topic_name,
                                              message, message_size);

    case SUPLA_CHANNELFNC_OPENINGSENSOR_GATEWAY:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_GATE:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_GARAGEDOOR:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_ROLLERSHUTTER:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_ROOFWINDOW:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_DOOR:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_WINDOW:
    case SUPLA_CHANNELFNC_MAILSENSOR:
    case SUPLA_CHANNELFNC_NOLIQUIDSENSOR:
      return get_sensor_message_at_index(index, topic_prefix, topic_name,
                                         message, message_size);

    case SUPLA_CHANNELFNC_THERMOMETER:
      return get_temperature_message_at_index(
          index, 1, topic_prefix, topic_name, message, message_size);

    case SUPLA_CHANNELFNC_HUMIDITY:
      return get_humidity_message_at_index(index, 1, topic_prefix, topic_name,
                                           message, message_size);

    case SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE:
      switch (index) {
        case 1:
          return get_humidity_message_at_index(
              index, 1, topic_prefix, topic_name, message, message_size);
        case 2:
          return get_temperature_message_at_index(
              index, 2, topic_prefix, topic_name, message, message_size);
      }
      break;
    case SUPLA_CHANNELFNC_DIMMER:
      switch (index) {
        case 1: {
          supla_channel_rgbw_value *rgbw_val =
              dynamic_cast<supla_channel_rgbw_value *>(channel_value);

          return get_onoff_message_at_index(
              rgbw_val && rgbw_val->get_brightness() > 0, index, topic_prefix,
              topic_name, message, message_size);
        }

        case 2:
          return get_brightness_message(topic_prefix, topic_name, message,
                                        message_size);
      }
      break;
    case SUPLA_CHANNELFNC_RGBLIGHTING:
      switch (index) {
        case 1: {
          supla_channel_rgbw_value *rgbw_val =
              dynamic_cast<supla_channel_rgbw_value *>(channel_value);

          return get_onoff_message_at_index(
              rgbw_val && rgbw_val->get_color_brightness() > 0, index,
              topic_prefix, topic_name, message, message_size);
        }

        case 2:
          return get_color_message(topic_prefix, topic_name, message,
                                   message_size);

        case 3:
          return get_normalized_rgb_message(topic_prefix, topic_name, message,
                                            message_size);

        case 4:
          return get_color_brightness_message(topic_prefix, topic_name, message,
                                              message_size);
      }
      break;
    case SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING:
      switch (index) {
        case 1: {
          supla_channel_rgbw_value *rgbw_val =
              dynamic_cast<supla_channel_rgbw_value *>(channel_value);

          return get_onoff_message_at_index(
              rgbw_val && (rgbw_val->get_brightness() > 0 ||
                           rgbw_val->get_color_brightness() > 0),
              index, topic_prefix, topic_name, message, message_size);
        }

        case 2:
          return get_color_message(topic_prefix, topic_name, message,
                                   message_size);
        case 3:
          return get_color_brightness_message(topic_prefix, topic_name, message,
                                              message_size);
        case 4:
          return get_brightness_message(topic_prefix, topic_name, message,
                                        message_size);
      }
      break;
    case SUPLA_CHANNELFNC_VALVE_OPENCLOSE:
    case SUPLA_CHANNELFNC_VALVE_PERCENTAGE:
      return get_valve_message_at_index(index, topic_prefix, topic_name,
                                        message, message_size);

    case SUPLA_CHANNELFNC_ELECTRICITY_METER:
      return get_electricitymeter_message_at_index(
          index, topic_prefix, topic_name, message, message_size);

    case SUPLA_CHANNELFNC_IC_ELECTRICITY_METER:
    case SUPLA_CHANNELFNC_IC_GAS_METER:
    case SUPLA_CHANNELFNC_IC_WATER_METER:
    case SUPLA_CHANNELFNC_IC_HEAT_METER:
      return get_impulsecounter_message_at_index(
          index, topic_prefix, topic_name, message, message_size);
  }

  return false;
}
