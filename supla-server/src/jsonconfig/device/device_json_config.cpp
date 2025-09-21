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

#include "device_json_config.h"

#include <string.h>

#include "log.h"

using std::map;
using std::string;

const map<unsigned _supla_int16_t, string> device_json_config::field_map = {
    {SUPLA_DEVICE_CONFIG_FIELD_STATUS_LED, "statusLed"},
    {SUPLA_DEVICE_CONFIG_FIELD_SCREEN_BRIGHTNESS, "screenBrightness"},
    {SUPLA_DEVICE_CONFIG_FIELD_BUTTON_VOLUME, "buttonVolume"},
    {SUPLA_DEVICE_CONFIG_FIELD_DISABLE_USER_INTERFACE, "userInterface"},
    {SUPLA_DEVICE_CONFIG_FIELD_AUTOMATIC_TIME_SYNC, "automaticTimeSync"},
    {SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_OFF_DELAY, "offDelay"},
    {SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_CONTENT, "content"},
    {SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_OFF_DELAY_TYPE, "offDelayType"},
    {SUPLA_DEVICE_CONFIG_FIELD_POWER_STATUS_LED, "powerStatusLed"},
    {SUPLA_DEVICE_CONFIG_FIELD_MODBUS, "modbus"},
    {SUPLA_DEVICE_CONFIG_FIELD_FIRMWARE_UPDATE, "firmwareUpdatePolicy"}};

const map<unsigned _supla_int16_t, string>
    device_json_config::home_screen_content_map = {
        {SUPLA_DEVCFG_HOME_SCREEN_CONTENT_NONE, "NONE"},
        {SUPLA_DEVCFG_HOME_SCREEN_CONTENT_TEMPERATURE, "TEMPERATURE"},
        {SUPLA_DEVCFG_HOME_SCREEN_CONTENT_TEMPERATURE_AND_HUMIDITY,
         "TEMPERATURE_AND_HUMIDITY"},
        {SUPLA_DEVCFG_HOME_SCREEN_CONTENT_TIME, "TIME"},
        {SUPLA_DEVCFG_HOME_SCREEN_CONTENT_TIME_DATE, "TIME_DATE"},
        {SUPLA_DEVCFG_HOME_SCREEN_CONTENT_TEMPERATURE_TIME, "TEMPERATURE_TIME"},
        {SUPLA_DEVCFG_HOME_SCREEN_CONTENT_MAIN_AND_AUX_TEMPERATURE,
         "MAIN_AND_AUX_TEMPERATURE"},
        {SUPLA_DEVCFG_HOME_SCREEN_CONTENT_MODE_OR_TEMPERATURE,
         "MODE_OR_TEMPERATURE"}};

#define SUPLA_DEVICE_CONFIG_FIELD_MODBUS_ROLE 1
#define SUPLA_DEVICE_CONFIG_FIELD_MODBUS_ADDRESS 2
#define SUPLA_DEVICE_CONFIG_FIELD_MODBUS_SLAVE_TIMEOUT_MS 3
#define SUPLA_DEVICE_CONFIG_FIELD_MODBUS_SERIAL_CONFIG 4
#define SUPLA_DEVICE_CONFIG_FIELD_MODBUS_NETWORK_CONFIG 5
#define SUPLA_DEVICE_CONFIG_FIELD_MODBUS_MODE 6
#define SUPLA_DEVICE_CONFIG_FIELD_MODBUS_BAUDRATE 7
#define SUPLA_DEVICE_CONFIG_FIELD_MODBUS_STOP_BITS 8
#define SUPLA_DEVICE_CONFIG_FIELD_MODBUS_PORT 9
#define SUPLA_DEVICE_CONFIG_FIELD_MODBUS_AVAILABLE_PROTOCOLS 10
#define SUPLA_DEVICE_CONFIG_FIELD_MODBUS_AVAILABLE_BAUDRATES 11
#define SUPLA_DEVICE_CONFIG_FIELD_MODBUS_AVAILABLE_STOP_BITS 12

const map<unsigned _supla_int16_t, string>
    device_json_config::modbus_field_map = {
        {SUPLA_DEVICE_CONFIG_FIELD_MODBUS_ROLE, "role"},
        {SUPLA_DEVICE_CONFIG_FIELD_MODBUS_ADDRESS, "modbusAddress"},
        {SUPLA_DEVICE_CONFIG_FIELD_MODBUS_SLAVE_TIMEOUT_MS, "slaveTimeoutMs"},
        {SUPLA_DEVICE_CONFIG_FIELD_MODBUS_SERIAL_CONFIG, "serialConfig"},
        {SUPLA_DEVICE_CONFIG_FIELD_MODBUS_NETWORK_CONFIG, "networkConfig"},
        {SUPLA_DEVICE_CONFIG_FIELD_MODBUS_MODE, "mode"},
        {SUPLA_DEVICE_CONFIG_FIELD_MODBUS_BAUDRATE, "baudRate"},
        {SUPLA_DEVICE_CONFIG_FIELD_MODBUS_STOP_BITS, "stopBits"},
        {SUPLA_DEVICE_CONFIG_FIELD_MODBUS_PORT, "port"},
        {SUPLA_DEVICE_CONFIG_FIELD_MODBUS_AVAILABLE_PROTOCOLS,
         "availableProtocols"},
        {SUPLA_DEVICE_CONFIG_FIELD_MODBUS_AVAILABLE_BAUDRATES,
         "availableBaudrates"},
        {SUPLA_DEVICE_CONFIG_FIELD_MODBUS_AVAILABLE_STOP_BITS,
         "availableStopbits"}};

const char device_json_config::content_available[] =
    "homeScreenContentAvailable";

const char device_json_config::disabled_str[] = "disabled";

const char device_json_config::min_allowed_temperature[] =
    "minAllowedTemperatureSetpointFromLocalUI";

const char device_json_config::max_allowed_temperature[] =
    "maxAllowedTemperatureSetpointFromLocalUI";

const char device_json_config::level_str[] = "level";
const char device_json_config::adjustment_str[] = "adjustment";

device_json_config::device_json_config(void) : supla_json_config() {}

device_json_config::device_json_config(supla_json_config *root)
    : supla_json_config(root) {}

device_json_config::~device_json_config(void) {}

string device_json_config::status_led_to_string(unsigned char status) {
  switch (status) {
    case SUPLA_DEVCFG_STATUS_LED_OFF_WHEN_CONNECTED:
      return "OFF_WHEN_CONNECTED";
    case SUPLA_DEVCFG_STATUS_LED_ALWAYS_OFF:
      return "ALWAYS_OFF";
  }

  return "ON_WHEN_CONNECTED";
}

unsigned char device_json_config::string_to_status_led(const string &status) {
  if (status == "OFF_WHEN_CONNECTED") {
    return SUPLA_DEVCFG_STATUS_LED_OFF_WHEN_CONNECTED;
  } else if (status == "ALWAYS_OFF") {
    return SUPLA_DEVCFG_STATUS_LED_ALWAYS_OFF;
  }

  return SUPLA_DEVCFG_STATUS_LED_ON_WHEN_CONNECTED;
}

string device_json_config::power_status_led_to_string(unsigned char status) {
  return status ? "DISABLED" : "ENABLED";
}

unsigned char device_json_config::string_to_power_status_led(
    const string &status) {
  return status == "DISABLED" ? 1 : 0;
}

string device_json_config::home_screen_content_to_string(
    unsigned char content) {
  for (auto it = home_screen_content_map.cbegin();
       it != home_screen_content_map.cend(); ++it) {
    if (it->first == content) {
      return it->second;
    }
  }

  return "";
}

unsigned char device_json_config::string_to_home_screen_content(
    const string &content) {
  for (auto it = home_screen_content_map.cbegin();
       it != home_screen_content_map.cend(); ++it) {
    if (it->second == content) {
      return it->first;
    }
  }

  return SUPLA_DEVCFG_HOME_SCREEN_CONTENT_NONE;
}

void device_json_config::set_status_led(TDeviceConfig_StatusLed *status_led) {
  if (status_led && status_led->StatusLedType >= 0 &&
      status_led->StatusLedType <= 2) {
    set_item_value(get_user_root(),
                   field_map.at(SUPLA_DEVICE_CONFIG_FIELD_STATUS_LED),
                   cJSON_String, true, nullptr,
                   status_led_to_string(status_led->StatusLedType).c_str(), 0);
  }
}

void device_json_config::set_power_status_led(
    TDeviceConfig_PowerStatusLed *status_led) {
  if (status_led && status_led->Disabled >= 0 && status_led->Disabled <= 1) {
    set_item_value(get_user_root(),
                   field_map.at(SUPLA_DEVICE_CONFIG_FIELD_POWER_STATUS_LED),
                   cJSON_String, true, nullptr,
                   power_status_led_to_string(status_led->Disabled).c_str(), 0);
  }
}

void device_json_config::set_screen_brightness(
    TDeviceConfig_ScreenBrightness *brightness) {
  if (brightness) {
    cJSON *br = cJSON_CreateObject();
    if (br) {
      if (brightness->Automatic == 1) {
        set_item_value(br, level_str, cJSON_String, true, nullptr, "auto", 0);
        set_item_value(br, adjustment_str, cJSON_Number, true, nullptr, nullptr,
                       brightness->AdjustmentForAutomatic);
      } else if (brightness->ScreenBrightness >= 0 &&
                 brightness->ScreenBrightness <= 100) {
        set_item_value(br, level_str, cJSON_Number, true, nullptr, nullptr,
                       brightness->ScreenBrightness);
      }

      set_item_value(get_user_root(),
                     field_map.at(SUPLA_DEVICE_CONFIG_FIELD_SCREEN_BRIGHTNESS),
                     cJSON_Object, true, br, nullptr, 0);
    }
  }
}

void device_json_config::set_button_volume(TDeviceConfig_ButtonVolume *volume) {
  unsigned char v = volume ? volume->Volume : 0;

  if (v <= 100) {
    set_item_value(get_user_root(),
                   field_map.at(SUPLA_DEVICE_CONFIG_FIELD_BUTTON_VOLUME),
                   cJSON_Number, true, nullptr, nullptr, v);
  }
}

void device_json_config::set_user_interface_disabled(
    TDeviceConfig_DisableUserInterface *disabled) {
  if (disabled && disabled->DisableUserInterface >= 0 &&
      disabled->DisableUserInterface <= 2) {
    cJSON *ui = cJSON_CreateObject();
    if (ui) {
      if (disabled->DisableUserInterface == 2) {
        cJSON_AddStringToObject(ui, disabled_str, "partial");
        cJSON_AddNumberToObject(
            ui, min_allowed_temperature,
            disabled->minAllowedTemperatureSetpointFromLocalUI);
        cJSON_AddNumberToObject(
            ui, max_allowed_temperature,
            disabled->maxAllowedTemperatureSetpointFromLocalUI);
      } else {
        cJSON_AddBoolToObject(ui, disabled_str,
                              disabled->DisableUserInterface ? true : false);
      }

      set_item_value(
          get_user_root(),
          field_map.at(SUPLA_DEVICE_CONFIG_FIELD_DISABLE_USER_INTERFACE),
          cJSON_Object, true, ui, nullptr, 0);
    }
  }
}

void device_json_config::set_automatic_time_sync(
    TDeviceConfig_AutomaticTimeSync *time_sync) {
  if (time_sync && (time_sync->AutomaticTimeSync == 0 ||
                    time_sync->AutomaticTimeSync == 1)) {
    set_item_value(get_user_root(),
                   field_map.at(SUPLA_DEVICE_CONFIG_FIELD_AUTOMATIC_TIME_SYNC),
                   time_sync->AutomaticTimeSync ? cJSON_True : cJSON_False,
                   true, nullptr, nullptr, 0);
  }
}

string device_json_config::modbus_role_to_string(unsigned char role) {
  switch (role) {
    case MODBUS_ROLE_MASTER:
      return "MASTER";
    case MODBUS_ROLE_SLAVE:
      return "SLAVE";
  }
  return "NOT_SET";
}

unsigned char device_json_config::string_to_modbus_role(const string &role) {
  if (role == "MASTER") {
    return MODBUS_ROLE_MASTER;
  } else if (role == "SLAVE") {
    return MODBUS_ROLE_SLAVE;
  }

  return MODBUS_ROLE_NOT_SET;
}

string device_json_config::modbus_serial_mode_to_string(unsigned char mode) {
  switch (mode) {
    case MODBUS_SERIAL_MODE_RTU:
      return "RTU";
    case MODBUS_SERIAL_MODE_ASCII:
      return "ASCII";
  }
  return "DISABLED";
}

unsigned char device_json_config::string_to_modbus_serial_mode(
    const string &mode) {
  if (mode == "RTU") {
    return MODBUS_SERIAL_MODE_RTU;
  } else if (mode == "ASCII") {
    return MODBUS_SERIAL_MODE_ASCII;
  }

  return MODBUS_SERIAL_MODE_DISABLED;
}

string device_json_config::modbus_network_mode_to_string(unsigned char mode) {
  switch (mode) {
    case MODBUS_NETWORK_MODE_TCP:
      return "TCP";
    case MODBUS_NETWORK_MODE_UDP:
      return "UDP";
  }
  return "DISABLED";
}

unsigned char device_json_config::string_to_modbus_network_mode(
    const string &mode) {
  if (mode == "TCP") {
    return MODBUS_NETWORK_MODE_TCP;
  } else if (mode == "UDP") {
    return MODBUS_NETWORK_MODE_UDP;
  }

  return MODBUS_NETWORK_MODE_DISABLED;
}

string device_json_config::modbus_stop_bits_to_string(unsigned char stop_bits) {
  switch (stop_bits) {
    case MODBUS_SERIAL_STOP_BITS_ONE_AND_HALF:
      return "ONE_AND_HALF";
    case MODBUS_SERIAL_STOP_BITS_TWO:
      return "TWO";
  }
  return "ONE";
}

unsigned char device_json_config::string_to_modbus_stop_bits(
    const string &stop_bits) {
  if (stop_bits == "ONE_AND_HALF") {
    return MODBUS_SERIAL_STOP_BITS_ONE_AND_HALF;
  } else if (stop_bits == "TWO") {
    return MODBUS_SERIAL_STOP_BITS_TWO;
  }

  return MODBUS_SERIAL_STOP_BITS_ONE;
}

cJSON *device_json_config::modbus_serial_config_to_json(
    ModbusSerialConfig *cfg) {
  cJSON *object = cJSON_CreateObject();

  set_item_value(object,
                 modbus_field_map.at(SUPLA_DEVICE_CONFIG_FIELD_MODBUS_MODE),
                 cJSON_String, true, nullptr,
                 modbus_serial_mode_to_string(cfg->Mode).c_str(), 0);

  set_item_value(object,
                 modbus_field_map.at(SUPLA_DEVICE_CONFIG_FIELD_MODBUS_BAUDRATE),
                 cJSON_Number, true, nullptr, nullptr, cfg->Baudrate);

  set_item_value(
      object, modbus_field_map.at(SUPLA_DEVICE_CONFIG_FIELD_MODBUS_STOP_BITS),
      cJSON_String, true, nullptr,
      modbus_stop_bits_to_string(cfg->StopBits).c_str(), 0);

  return object;
}

bool device_json_config::modbus_json_to_serial_config(cJSON *json,
                                                      ModbusSerialConfig *cfg) {
  bool result = false;

  if (!json || !cfg) {
    return result;
  }

  string str_value;

  if (get_string(
          json,
          modbus_field_map.at(SUPLA_DEVICE_CONFIG_FIELD_MODBUS_MODE).c_str(),
          &str_value)) {
    cfg->Mode = string_to_modbus_serial_mode(str_value);
    result = true;
  }

  double dbl_value = 0;

  if (get_double(json,
                 modbus_field_map.at(SUPLA_DEVICE_CONFIG_FIELD_MODBUS_BAUDRATE)
                     .c_str(),
                 &dbl_value)) {
    cfg->Baudrate = dbl_value;
  }

  if (get_string(json,
                 modbus_field_map.at(SUPLA_DEVICE_CONFIG_FIELD_MODBUS_STOP_BITS)
                     .c_str(),
                 &str_value)) {
    cfg->StopBits = string_to_modbus_stop_bits(str_value);
    result = true;
  }

  return result;
}

cJSON *device_json_config::modbus_available_protocols_to_json(
    ModbusConfigProperties *props) {
  cJSON *available_protocols = cJSON_CreateArray();

  if (props->Protocol.Master) {
    cJSON_AddItemToArray(
        available_protocols,
        cJSON_CreateString(modbus_role_to_string(MODBUS_ROLE_MASTER).c_str()));
  }

  if (props->Protocol.Slave) {
    cJSON_AddItemToArray(
        available_protocols,
        cJSON_CreateString(modbus_role_to_string(MODBUS_ROLE_SLAVE).c_str()));
  }

  if (props->Protocol.Rtu) {
    cJSON_AddItemToArray(
        available_protocols,
        cJSON_CreateString(
            modbus_serial_mode_to_string(MODBUS_SERIAL_MODE_RTU).c_str()));
  }

  if (props->Protocol.Ascii) {
    cJSON_AddItemToArray(
        available_protocols,
        cJSON_CreateString(
            modbus_serial_mode_to_string(MODBUS_SERIAL_MODE_ASCII).c_str()));
  }

  if (props->Protocol.Tcp) {
    cJSON_AddItemToArray(
        available_protocols,
        cJSON_CreateString(
            modbus_network_mode_to_string(MODBUS_NETWORK_MODE_TCP).c_str()));
  }

  if (props->Protocol.Udp) {
    cJSON_AddItemToArray(
        available_protocols,
        cJSON_CreateString(
            modbus_network_mode_to_string(MODBUS_NETWORK_MODE_UDP).c_str()));
  }

  return available_protocols;
}

bool device_json_config::modbus_json_to_available_protocols(
    cJSON *json, ModbusConfigProperties *props) {
  memset(&props->Protocol, 0, sizeof(props->Protocol));

  cJSON *available_protocols = cJSON_GetObjectItem(
      json,
      modbus_field_map.at(SUPLA_DEVICE_CONFIG_FIELD_MODBUS_AVAILABLE_PROTOCOLS)
          .c_str());

  if (!available_protocols) {
    return false;
  }

  for (int a = 0; a < cJSON_GetArraySize(available_protocols); a++) {
    cJSON *item = cJSON_GetArrayItem(available_protocols, a);
    if (item && cJSON_IsString(item)) {
      const char *value = cJSON_GetStringValue(item);
      if (modbus_role_to_string(MODBUS_ROLE_MASTER) == value) {
        props->Protocol.Master = 1;
      } else if (modbus_role_to_string(MODBUS_ROLE_SLAVE) == value) {
        props->Protocol.Slave = 1;
      } else if (modbus_serial_mode_to_string(MODBUS_SERIAL_MODE_RTU) ==
                 value) {
        props->Protocol.Rtu = 1;
      } else if (modbus_serial_mode_to_string(MODBUS_SERIAL_MODE_ASCII) ==
                 value) {
        props->Protocol.Ascii = 1;
      } else if (modbus_network_mode_to_string(MODBUS_NETWORK_MODE_TCP) ==
                 value) {
        props->Protocol.Tcp = 1;
      } else if (modbus_network_mode_to_string(MODBUS_NETWORK_MODE_UDP) ==
                 value) {
        props->Protocol.Udp = 1;
      }
    }
  }

  return true;
}

cJSON *device_json_config::modbus_available_baudrates_to_json(
    ModbusConfigProperties *props) {
  cJSON *available_baudrates = cJSON_CreateArray();

  if (props->Baudrate.B4800) {
    cJSON_AddItemToArray(available_baudrates, cJSON_CreateNumber(4800));
  }

  if (props->Baudrate.B9600) {
    cJSON_AddItemToArray(available_baudrates, cJSON_CreateNumber(9600));
  }

  if (props->Baudrate.B19200) {
    cJSON_AddItemToArray(available_baudrates, cJSON_CreateNumber(19200));
  }

  if (props->Baudrate.B38400) {
    cJSON_AddItemToArray(available_baudrates, cJSON_CreateNumber(38400));
  }

  if (props->Baudrate.B57600) {
    cJSON_AddItemToArray(available_baudrates, cJSON_CreateNumber(57600));
  }

  if (props->Baudrate.B115200) {
    cJSON_AddItemToArray(available_baudrates, cJSON_CreateNumber(115200));
  }

  return available_baudrates;
}

bool device_json_config::modbus_json_to_available_baudrates(
    cJSON *json, ModbusConfigProperties *props) {
  memset(&props->Baudrate, 0, sizeof(props->Baudrate));

  cJSON *available_baudrates = cJSON_GetObjectItem(
      json,
      modbus_field_map.at(SUPLA_DEVICE_CONFIG_FIELD_MODBUS_AVAILABLE_BAUDRATES)
          .c_str());

  if (!available_baudrates) {
    return false;
  }

  for (int a = 0; a < cJSON_GetArraySize(available_baudrates); a++) {
    cJSON *item = cJSON_GetArrayItem(available_baudrates, a);
    if (item && cJSON_IsNumber(item)) {
      int value = cJSON_GetNumberValue(item);
      switch (value) {
        case 4800:
          props->Baudrate.B4800 = 1;
          break;
        case 9600:
          props->Baudrate.B9600 = 1;
          break;
        case 19200:
          props->Baudrate.B19200 = 1;
          break;
        case 38400:
          props->Baudrate.B38400 = 1;
          break;
        case 57600:
          props->Baudrate.B57600 = 1;
          break;
        case 115200:
          props->Baudrate.B115200 = 1;
          break;
      }
    }
  }

  return true;
}

cJSON *device_json_config::modbus_available_stop_bits_to_json(
    ModbusConfigProperties *props) {
  cJSON *available_stopbits = cJSON_CreateArray();
  if (props->StopBits.One) {
    cJSON_AddItemToArray(
        available_stopbits,
        cJSON_CreateString(
            modbus_stop_bits_to_string(MODBUS_SERIAL_STOP_BITS_ONE).c_str()));
  }

  if (props->StopBits.Two) {
    cJSON_AddItemToArray(
        available_stopbits,
        cJSON_CreateString(
            modbus_stop_bits_to_string(MODBUS_SERIAL_STOP_BITS_TWO).c_str()));
  }

  if (props->StopBits.OneAndHalf) {
    cJSON_AddItemToArray(
        available_stopbits,
        cJSON_CreateString(
            modbus_stop_bits_to_string(MODBUS_SERIAL_STOP_BITS_ONE_AND_HALF)
                .c_str()));
  }
  return available_stopbits;
}

bool device_json_config::modbus_json_to_available_stop_bits(
    cJSON *json, ModbusConfigProperties *props) {
  memset(&props->StopBits, 0, sizeof(props->StopBits));

  cJSON *available_stop_bits = cJSON_GetObjectItem(
      json,
      modbus_field_map.at(SUPLA_DEVICE_CONFIG_FIELD_MODBUS_AVAILABLE_STOP_BITS)
          .c_str());

  if (!available_stop_bits) {
    return false;
  }

  for (int a = 0; a < cJSON_GetArraySize(available_stop_bits); a++) {
    cJSON *item = cJSON_GetArrayItem(available_stop_bits, a);
    if (item && cJSON_IsString(item)) {
      const char *value = cJSON_GetStringValue(item);
      if (modbus_stop_bits_to_string(MODBUS_SERIAL_STOP_BITS_ONE) == value) {
        props->StopBits.One = 1;
      } else if (modbus_stop_bits_to_string(MODBUS_SERIAL_STOP_BITS_TWO) ==
                 value) {
        props->StopBits.Two = 1;
      } else if (modbus_stop_bits_to_string(
                     MODBUS_SERIAL_STOP_BITS_ONE_AND_HALF) == value) {
        props->StopBits.OneAndHalf = 1;
      }
    }
  }

  return true;
}

cJSON *device_json_config::modbus_network_config_to_json(
    ModbusNetworkConfig *cfg) {
  cJSON *object = cJSON_CreateObject();

  set_item_value(object,
                 modbus_field_map.at(SUPLA_DEVICE_CONFIG_FIELD_MODBUS_MODE),
                 cJSON_String, true, nullptr,
                 modbus_network_mode_to_string(cfg->Mode).c_str(), 0);

  set_item_value(object,
                 modbus_field_map.at(SUPLA_DEVICE_CONFIG_FIELD_MODBUS_PORT),
                 cJSON_Number, true, nullptr, nullptr, cfg->Port);

  return object;
}

bool device_json_config::modbus_json_to_network_config(
    cJSON *json, ModbusNetworkConfig *cfg) {
  bool result = false;

  if (!json || !cfg) {
    return result;
  }

  string str_value;

  if (get_string(
          json,
          modbus_field_map.at(SUPLA_DEVICE_CONFIG_FIELD_MODBUS_MODE).c_str(),
          &str_value)) {
    cfg->Mode = string_to_modbus_network_mode(str_value);
    result = true;
  }

  double dbl_value = 0;

  if (get_double(
          json,
          modbus_field_map.at(SUPLA_DEVICE_CONFIG_FIELD_MODBUS_PORT).c_str(),
          &dbl_value)) {
    cfg->Port = dbl_value;
  }

  return result;
}

void device_json_config::set_modbus_config(TDeviceConfig_Modbus *cfg) {
  if (!cfg) {
    return;
  }

  cJSON *user_root = get_modbus_root(true, true);
  if (!user_root) {
    return;
  }

  set_item_value(
      user_root, modbus_field_map.at(SUPLA_DEVICE_CONFIG_FIELD_MODBUS_ROLE),
      cJSON_String, true, nullptr, modbus_role_to_string(cfg->Role).c_str(), 0);

  set_item_value(user_root,
                 modbus_field_map.at(SUPLA_DEVICE_CONFIG_FIELD_MODBUS_ADDRESS),
                 cJSON_Number, true, nullptr, nullptr, cfg->ModbusAddress);

  set_item_value(
      user_root,
      modbus_field_map.at(SUPLA_DEVICE_CONFIG_FIELD_MODBUS_SLAVE_TIMEOUT_MS),
      cJSON_Number, true, nullptr, nullptr, cfg->SlaveTimeoutMs);

  set_item_value(
      user_root,
      modbus_field_map.at(SUPLA_DEVICE_CONFIG_FIELD_MODBUS_SERIAL_CONFIG),
      cJSON_Object, true, modbus_serial_config_to_json(&cfg->SerialConfig),
      nullptr, 0);

  set_item_value(
      user_root,
      modbus_field_map.at(SUPLA_DEVICE_CONFIG_FIELD_MODBUS_NETWORK_CONFIG),
      cJSON_Object, true, modbus_network_config_to_json(&cfg->NetworkConfig),
      nullptr, 0);

  cJSON *properties_root = get_modbus_root(true, false);
  if (!properties_root) {
    return;
  }

  set_item_value(
      properties_root,
      modbus_field_map.at(SUPLA_DEVICE_CONFIG_FIELD_MODBUS_AVAILABLE_PROTOCOLS),
      cJSON_Object, true, modbus_available_protocols_to_json(&cfg->Properties),
      nullptr, 0);

  set_item_value(
      properties_root,
      modbus_field_map.at(SUPLA_DEVICE_CONFIG_FIELD_MODBUS_AVAILABLE_BAUDRATES),
      cJSON_Object, true, modbus_available_baudrates_to_json(&cfg->Properties),
      nullptr, 0);

  set_item_value(
      properties_root,
      modbus_field_map.at(SUPLA_DEVICE_CONFIG_FIELD_MODBUS_AVAILABLE_STOP_BITS),
      cJSON_Object, true, modbus_available_stop_bits_to_json(&cfg->Properties),
      nullptr, 0);
}

string device_json_config::fw_update_policy_to_string(unsigned char policy) {
  switch (policy) {
    case SUPLA_FIRMWARE_UPDATE_POLICY_FORCED_OFF:
      return "FORCED_OFF";
    case SUPLA_FIRMWARE_UPDATE_POLICY_DISABLED:
      return "DISABLED";
    case SUPLA_FIRMWARE_UPDATE_POLICY_ALL_ENABLED:
      return "ALL_ENABLED";
  }

  return "SECURITY_ONLY";
}

void device_json_config::set_firmware_update_config(
    TDeviceConfig_FirmwareUpdate *cfg) {
  if (cfg) {
    set_item_value(get_user_root(),
                   field_map.at(SUPLA_DEVICE_CONFIG_FIELD_FIRMWARE_UPDATE),
                   cJSON_String, true, nullptr,
                   fw_update_policy_to_string(cfg->Policy).c_str(), 0);
  }
}

cJSON *device_json_config::get_root(bool create, bool user,
                                    unsigned _supla_int64_t field) {
  cJSON *root = user ? get_user_root() : get_properties_root();

  if (field != SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_OFF_DELAY &&
      field != SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_CONTENT &&
      field != SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_OFF_DELAY_TYPE) {
    return root;
  }

  if (root) {
    const char name[] = "homeScreen";

    cJSON *result = cJSON_GetObjectItem(root, name);
    if (!result && create) {
      result = cJSON_AddObjectToObject(root, name);
    }

    return result;
  }
  return nullptr;
}

void device_json_config::set_home_screen_off_delay(
    TDeviceConfig_HomeScreenOffDelay *delay) {
  if (delay) {
    set_item_value(
        get_root(true, true, SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_OFF_DELAY),
        field_map.at(SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_OFF_DELAY),
        cJSON_Number, true, nullptr, nullptr, delay->HomeScreenOffDelayS);
  }
}

void device_json_config::set_home_screen_off_delay_type(
    TDeviceConfig_HomeScreenOffDelayType *type) {
  if (type) {
    set_item_value(
        get_root(true, true,
                 SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_OFF_DELAY_TYPE),
        field_map.at(SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_OFF_DELAY_TYPE),
        cJSON_String, true, nullptr,
        type->HomeScreenOffDelayType ==
                SUPLA_DEVCFG_HOME_SCREEN_OFF_DELAY_TYPE_ENABLED_WHEN_DARK
            ? "ENABLED_WHEN_DARK"
            : "ALWAYS_ENABLED",
        0);
  }
}

void device_json_config::set_home_screen_content(
    TDeviceConfig_HomeScreenContent *content) {
  cJSON *root =
      get_root(true, true, SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_CONTENT);
  string content_str =
      home_screen_content_to_string(content->HomeScreenContent);
  if (!content_str.empty()) {
    set_item_value(root,
                   field_map.at(SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_CONTENT),
                   cJSON_String, true, nullptr, content_str.c_str(), 0);
  }

  cJSON *content_items =
      cJSON_GetObjectItem(get_properties_root(), content_available);

  if (content_items) {
    cJSON_DetachItemViaPointer(get_properties_root(), content_items);
    cJSON_Delete(content_items);
  }

  content_items =
      cJSON_AddArrayToObject(get_properties_root(), content_available);
  if (content_items) {
    unsigned char size = sizeof(content->ContentAvailable) * 8;
    unsigned _supla_int64_t n = 1;
    for (unsigned char a = 0; a < size; a++) {
      if (content->ContentAvailable & n) {
        content_str = home_screen_content_to_string(n);
        if (!content_str.empty()) {
          cJSON *mode_json = cJSON_CreateString(content_str.c_str());
          if (mode_json) {
            cJSON_AddItemToArray(content_items, mode_json);
          }
        }
      }
      n <<= 1;
    }
  }
}

void device_json_config::set_config(TSDS_SetDeviceConfig *config) {
  if (!config) {
    return;
  }

  unsigned short offset = 0;

  // map is sorted
  for (auto it = field_map.cbegin(); it != field_map.cend(); ++it) {
    if (config->Fields & it->first) {
      unsigned _supla_int16_t left = config->ConfigSize;
      if (left > SUPLA_DEVICE_CONFIG_MAXSIZE) {
        left = SUPLA_DEVICE_CONFIG_MAXSIZE;
      }
      if (left > offset) {
        left -= offset;
      } else {
        return;
      }

      void *ptr = &config->Config[offset];
      unsigned short size = 0;

      switch (it->first) {
        case SUPLA_DEVICE_CONFIG_FIELD_STATUS_LED:
          if (left >= (size = sizeof(TDeviceConfig_StatusLed))) {
            set_status_led(static_cast<TDeviceConfig_StatusLed *>(ptr));
          }
          break;
        case SUPLA_DEVICE_CONFIG_FIELD_SCREEN_BRIGHTNESS:
          if (left >= (size = sizeof(TDeviceConfig_ScreenBrightness))) {
            set_screen_brightness(
                static_cast<TDeviceConfig_ScreenBrightness *>(ptr));
          }
          break;
        case SUPLA_DEVICE_CONFIG_FIELD_BUTTON_VOLUME:
          if (left >= (size = sizeof(TDeviceConfig_ButtonVolume))) {
            set_button_volume(static_cast<TDeviceConfig_ButtonVolume *>(ptr));
          }
          break;
        case SUPLA_DEVICE_CONFIG_FIELD_DISABLE_USER_INTERFACE:
          if (left >= (size = sizeof(TDeviceConfig_DisableUserInterface))) {
            set_user_interface_disabled(
                static_cast<TDeviceConfig_DisableUserInterface *>(ptr));
          }
          break;
        case SUPLA_DEVICE_CONFIG_FIELD_AUTOMATIC_TIME_SYNC:
          if (left >= (size = sizeof(TDeviceConfig_AutomaticTimeSync))) {
            set_automatic_time_sync(
                static_cast<TDeviceConfig_AutomaticTimeSync *>(ptr));
          }
          break;
        case SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_OFF_DELAY:
          if (left >= (size = sizeof(TDeviceConfig_HomeScreenOffDelay))) {
            set_home_screen_off_delay(
                static_cast<TDeviceConfig_HomeScreenOffDelay *>(ptr));
          }
          break;
        case SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_CONTENT:
          if (left >= (size = sizeof(TDeviceConfig_HomeScreenContent))) {
            set_home_screen_content(
                static_cast<TDeviceConfig_HomeScreenContent *>(ptr));
          }
          break;
        case SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_OFF_DELAY_TYPE:
          if (left >= (size = sizeof(TDeviceConfig_HomeScreenOffDelayType))) {
            set_home_screen_off_delay_type(
                static_cast<TDeviceConfig_HomeScreenOffDelayType *>(ptr));
          }
          break;
        case SUPLA_DEVICE_CONFIG_FIELD_POWER_STATUS_LED:
          if (left >= (size = sizeof(TDeviceConfig_PowerStatusLed))) {
            set_power_status_led(
                static_cast<TDeviceConfig_PowerStatusLed *>(ptr));
          }
          break;
        case SUPLA_DEVICE_CONFIG_FIELD_MODBUS:
          if (left >= (size = sizeof(TDeviceConfig_Modbus))) {
            set_modbus_config(static_cast<TDeviceConfig_Modbus *>(ptr));
          }
          break;
        case SUPLA_DEVICE_CONFIG_FIELD_FIRMWARE_UPDATE:
          if (left >= (size = sizeof(TDeviceConfig_FirmwareUpdate))) {
            set_firmware_update_config(
                static_cast<TDeviceConfig_FirmwareUpdate *>(ptr));
          }
          break;
      }

      offset += size;
    }
  }
}

bool device_json_config::get_status_led(TDeviceConfig_StatusLed *status_led) {
  string str_value;
  if (status_led &&
      get_string(get_user_root(),
                 field_map.at(SUPLA_DEVICE_CONFIG_FIELD_STATUS_LED).c_str(),
                 &str_value)) {
    status_led->StatusLedType = string_to_status_led(str_value);
    return true;
  }
  return false;
}

bool device_json_config::get_power_status_led(
    TDeviceConfig_PowerStatusLed *status_led) {
  string str_value;
  if (status_led &&
      get_string(
          get_user_root(),
          field_map.at(SUPLA_DEVICE_CONFIG_FIELD_POWER_STATUS_LED).c_str(),
          &str_value)) {
    status_led->Disabled = string_to_power_status_led(str_value);
    return true;
  }
  return false;
}

bool device_json_config::get_screen_brightness(
    TDeviceConfig_ScreenBrightness *brightness) {
  if (!brightness) {
    return false;
  }

  cJSON *parent = cJSON_GetObjectItem(
      get_user_root(),
      field_map.at(SUPLA_DEVICE_CONFIG_FIELD_SCREEN_BRIGHTNESS).c_str());

  if (parent) {
    double value = 0;
    if (get_double(parent, level_str, &value) && value >= 0 && value <= 100) {
      brightness->Automatic = 0;
      brightness->ScreenBrightness = value;
      return true;
    }

    string str_value;
    if (get_string(parent, level_str, &str_value) && str_value == "auto") {
      brightness->Automatic = 1;
      brightness->ScreenBrightness = 0;

      if (get_double(parent, adjustment_str, &value)) {
        brightness->AdjustmentForAutomatic = value;
      }

      return true;
    }
  }

  return false;
}

bool device_json_config::get_button_volume(TDeviceConfig_ButtonVolume *volume) {
  double value = 0;
  if (volume &&
      get_double(get_user_root(),
                 field_map.at(SUPLA_DEVICE_CONFIG_FIELD_BUTTON_VOLUME).c_str(),
                 &value) &&
      value >= 0 && value <= 100) {
    volume->Volume = value;
    return true;
  }

  return false;
}

bool device_json_config::get_user_interface_disabled(
    TDeviceConfig_DisableUserInterface *disabled) {
  bool bool_value = 0;
  double dbl_min = 0;
  double dbl_max = 0;
  string str_value;

  if (disabled) {
    cJSON *parent = cJSON_GetObjectItem(
        get_user_root(),
        field_map.at(SUPLA_DEVICE_CONFIG_FIELD_DISABLE_USER_INTERFACE).c_str());

    disabled->minAllowedTemperatureSetpointFromLocalUI = 0;
    disabled->maxAllowedTemperatureSetpointFromLocalUI = 0;

    if (get_bool(parent, disabled_str, &bool_value)) {
      disabled->DisableUserInterface = bool_value ? 1 : 0;
      return true;
    } else if (get_string(parent, disabled_str, &str_value) &&
               str_value == "partial" &&
               get_double(parent, min_allowed_temperature, &dbl_min) &&
               get_double(parent, max_allowed_temperature, &dbl_max)) {
      disabled->DisableUserInterface = 2;
      disabled->minAllowedTemperatureSetpointFromLocalUI = dbl_min;
      disabled->maxAllowedTemperatureSetpointFromLocalUI = dbl_max;
      return true;
    }
  }

  return false;
}

bool device_json_config::get_automatic_time_sync(
    TDeviceConfig_AutomaticTimeSync *time_sync) {
  bool value = 0;
  if (time_sync &&
      get_bool(
          get_user_root(),
          field_map.at(SUPLA_DEVICE_CONFIG_FIELD_AUTOMATIC_TIME_SYNC).c_str(),
          &value)) {
    time_sync->AutomaticTimeSync = value ? 1 : 0;
    return true;
  }

  return false;
}

bool device_json_config::get_home_screen_off_delay(
    TDeviceConfig_HomeScreenOffDelay *delay) {
  double value = 0;
  if (delay &&
      get_double(
          get_root(false, true,
                   SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_OFF_DELAY),
          field_map.at(SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_OFF_DELAY).c_str(),
          &value)) {
    delay->HomeScreenOffDelayS = value;
    return true;
  }

  return false;
}

bool device_json_config::get_home_screen_off_delay_type(
    TDeviceConfig_HomeScreenOffDelayType *type) {
  if (type) {
    string str_value;

    if (get_string(
            get_root(false, true,
                     SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_OFF_DELAY_TYPE),
            field_map.at(SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_OFF_DELAY_TYPE)
                .c_str(),
            &str_value)) {
      type->HomeScreenOffDelayType =
          str_value == "ENABLED_WHEN_DARK"
              ? SUPLA_DEVCFG_HOME_SCREEN_OFF_DELAY_TYPE_ENABLED_WHEN_DARK
              : SUPLA_DEVCFG_HOME_SCREEN_OFF_DELAY_TYPE_ALWAYS_ENABLED;

      return true;
    }
  }

  return false;
}

bool device_json_config::get_home_screen_content(
    TDeviceConfig_HomeScreenContent *content) {
  string str_value;
  bool result = false;

  if (content) {
    cJSON *root =
        get_root(false, true, SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_OFF_DELAY);
    if (get_string(
            root,
            field_map.at(SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_CONTENT).c_str(),
            &str_value)) {
      content->HomeScreenContent = string_to_home_screen_content(str_value);
      result = true;
    }

    content->ContentAvailable = 0;

    cJSON *modes =
        cJSON_GetObjectItem(get_properties_root(), content_available);

    if (modes && cJSON_IsArray(modes)) {
      result = true;
      for (int a = 0; a < cJSON_GetArraySize(modes); a++) {
        cJSON *item = cJSON_GetArrayItem(modes, a);
        if (item && cJSON_IsString(item)) {
          content->ContentAvailable |=
              string_to_home_screen_content(cJSON_GetStringValue(item));
        }
      }
    }
  }
  return result;
}

unsigned _supla_int64_t device_json_config::get_available_fields(void) {
  unsigned _supla_int64_t result = 0;
  for (auto it = field_map.cbegin(); it != field_map.cend(); ++it) {
    cJSON *root = get_root(false, true, it->first);
    if (root && cJSON_GetObjectItem(root, it->second.c_str())) {
      result |= it->first;
    }
  }
  return result;
}

cJSON *device_json_config::get_modbus_root(bool create, bool user) {
  cJSON *root = user ? get_user_root() : get_properties_root();
  cJSON *result = nullptr;
  if (root) {
    const char *name = field_map.at(SUPLA_DEVICE_CONFIG_FIELD_MODBUS).c_str();

    result = cJSON_GetObjectItem(root, name);

    if (!result && create) {
      result = cJSON_AddObjectToObject(root, name);
    }
  }
  return result;
}

bool device_json_config::get_modbus_config(TDeviceConfig_Modbus *cfg) {
  bool result = false;

  if (!cfg) {
    return result;
  }

  cJSON *user_root = get_modbus_root(false, true);
  if (!user_root) {
    return result;
  }

  string str_value;

  if (get_string(
          user_root,
          modbus_field_map.at(SUPLA_DEVICE_CONFIG_FIELD_MODBUS_ROLE).c_str(),
          &str_value)) {
    cfg->Role = string_to_modbus_role(str_value);
    result = true;
  }

  double dbl_value = 0;

  if (get_double(
          user_root,
          modbus_field_map.at(SUPLA_DEVICE_CONFIG_FIELD_MODBUS_ADDRESS).c_str(),
          &dbl_value)) {
    cfg->ModbusAddress = dbl_value;
    result = true;
  }

  if (get_double(
          user_root,
          modbus_field_map.at(SUPLA_DEVICE_CONFIG_FIELD_MODBUS_SLAVE_TIMEOUT_MS)
              .c_str(),
          &dbl_value)) {
    cfg->SlaveTimeoutMs = dbl_value;
    result = true;
  }

  if (modbus_json_to_serial_config(
          cJSON_GetObjectItem(
              user_root, modbus_field_map
                             .at(SUPLA_DEVICE_CONFIG_FIELD_MODBUS_SERIAL_CONFIG)
                             .c_str()),
          &cfg->SerialConfig)) {
    result = true;
  }

  if (modbus_json_to_network_config(
          cJSON_GetObjectItem(
              user_root,
              modbus_field_map
                  .at(SUPLA_DEVICE_CONFIG_FIELD_MODBUS_NETWORK_CONFIG)
                  .c_str()),
          &cfg->NetworkConfig)) {
    result = true;
  }

  cJSON *properties_root = get_modbus_root(false, false);
  if (!properties_root) {
    return result;
  }

  if (modbus_json_to_available_protocols(properties_root, &cfg->Properties)) {
    result = true;
  }

  if (modbus_json_to_available_baudrates(properties_root, &cfg->Properties)) {
    result = true;
  }

  if (modbus_json_to_available_stop_bits(properties_root, &cfg->Properties)) {
    result = true;
  }

  return result;
}

unsigned char device_json_config::string_to_fw_update_policy(
    const std::string &policy) {
  if (policy == "FORCED_OFF") {
    return SUPLA_FIRMWARE_UPDATE_POLICY_FORCED_OFF;
  } else if (policy == "DISABLED") {
    return SUPLA_FIRMWARE_UPDATE_POLICY_DISABLED;
  } else if (policy == "ALL_ENABLED") {
    return SUPLA_FIRMWARE_UPDATE_POLICY_ALL_ENABLED;
  }

  return SUPLA_FIRMWARE_UPDATE_POLICY_SECURITY_ONLY;
}

bool device_json_config::get_firmware_update_config(
    TDeviceConfig_FirmwareUpdate *cfg) {
  string str_value;
  if (cfg &&
      get_string(
          get_user_root(),
          field_map.at(SUPLA_DEVICE_CONFIG_FIELD_FIRMWARE_UPDATE).c_str(),
          &str_value)) {
    cfg->Policy = string_to_fw_update_policy(str_value);
    return true;
  }
  return false;
}

void device_json_config::get_config(TSDS_SetDeviceConfig *config,
                                    unsigned _supla_int64_t fields,
                                    unsigned _supla_int64_t *fields_left) {
  if (fields_left) {
    *fields_left = 0;
    // map is sorted
    for (auto it = field_map.cbegin(); it != field_map.cend(); ++it) {
      if (fields & it->first) {
        *fields_left |= it->first;
      }
    }
  }

  if (!config) {
    return;
  }

  config->Fields = 0;
  config->ConfigSize = 0;

  // map is sorted
  for (auto it = field_map.cbegin(); it != field_map.cend(); ++it) {
    if (fields & it->first) {
      void *ptr = &config->Config[config->ConfigSize];
      unsigned short size = 0;
      bool field_set = false;

      unsigned _supla_int16_t left =
          SUPLA_DEVICE_CONFIG_MAXSIZE - config->ConfigSize;

      switch (it->first) {
        case SUPLA_DEVICE_CONFIG_FIELD_STATUS_LED:
          field_set =
              left >= (size = sizeof(TDeviceConfig_StatusLed)) &&
              get_status_led(static_cast<TDeviceConfig_StatusLed *>(ptr));
          break;
        case SUPLA_DEVICE_CONFIG_FIELD_SCREEN_BRIGHTNESS:
          field_set = left >= (size = sizeof(TDeviceConfig_ScreenBrightness)) &&
                      get_screen_brightness(
                          static_cast<TDeviceConfig_ScreenBrightness *>(ptr));
          break;
        case SUPLA_DEVICE_CONFIG_FIELD_BUTTON_VOLUME:
          field_set =
              left >= (size = sizeof(TDeviceConfig_ButtonVolume)) &&
              get_button_volume(static_cast<TDeviceConfig_ButtonVolume *>(ptr));
          break;
        case SUPLA_DEVICE_CONFIG_FIELD_DISABLE_USER_INTERFACE:
          field_set =
              left >= (size = sizeof(TDeviceConfig_DisableUserInterface)) &&
              get_user_interface_disabled(
                  static_cast<TDeviceConfig_DisableUserInterface *>(ptr));
          break;
        case SUPLA_DEVICE_CONFIG_FIELD_AUTOMATIC_TIME_SYNC:
          field_set =
              left >= (size = sizeof(TDeviceConfig_AutomaticTimeSync)) &&
              get_automatic_time_sync(
                  static_cast<TDeviceConfig_AutomaticTimeSync *>(ptr));
          break;
        case SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_OFF_DELAY:
          field_set =
              left >= (size = sizeof(TDeviceConfig_HomeScreenOffDelay)) &&
              get_home_screen_off_delay(
                  static_cast<TDeviceConfig_HomeScreenOffDelay *>(ptr));
          break;
        case SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_CONTENT:
          field_set =
              left >= (size = sizeof(TDeviceConfig_HomeScreenContent)) &&
              get_home_screen_content(
                  static_cast<TDeviceConfig_HomeScreenContent *>(ptr));
          break;
        case SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_OFF_DELAY_TYPE:
          field_set =
              left >= (size = sizeof(TDeviceConfig_HomeScreenOffDelayType)) &&
              get_home_screen_off_delay_type(
                  static_cast<TDeviceConfig_HomeScreenOffDelayType *>(ptr));
          break;
        case SUPLA_DEVICE_CONFIG_FIELD_POWER_STATUS_LED:
          field_set = left >= (size = sizeof(TDeviceConfig_PowerStatusLed)) &&
                      get_power_status_led(
                          static_cast<TDeviceConfig_PowerStatusLed *>(ptr));
          break;
        case SUPLA_DEVICE_CONFIG_FIELD_MODBUS:
          field_set =
              left >= (size = sizeof(TDeviceConfig_Modbus)) &&
              get_modbus_config(static_cast<TDeviceConfig_Modbus *>(ptr));
          break;
        case SUPLA_DEVICE_CONFIG_FIELD_FIRMWARE_UPDATE:
          field_set = left >= (size = sizeof(TDeviceConfig_FirmwareUpdate)) &&
                      get_firmware_update_config(
                          static_cast<TDeviceConfig_FirmwareUpdate *>(ptr));
          break;
      }

      if (field_set) {
        config->Fields |= it->first;
        config->ConfigSize += size;
      } else if (size > left) {
        return;
      }

      if (fields_left) {
        *fields_left &= ~(it->first);
      }

      if (config->ConfigSize >= SUPLA_DEVICE_CONFIG_MAXSIZE) {
        return;
      }
    }
  }
}

void device_json_config::get_config(TSDS_SetDeviceConfig *config,
                                    unsigned _supla_int64_t *fields_left) {
  get_config(config, 0xFFFFFFFFFFFFFFFF, fields_left);
}

void device_json_config::remove_empty_sub_roots() {
  cJSON *root = get_user_root();
  for (auto it = field_map.cbegin(); it != field_map.cend(); ++it) {
    cJSON *item = get_root(false, true, it->first);
    if (item && root != item && !item->child) {
      cJSON_DetachItemViaPointer(root, item);
      cJSON_Delete(item);
    }
  }
}

void device_json_config::leave_only_thise_fields(
    unsigned _supla_int64_t fields) {
  for (auto it = field_map.cbegin(); it != field_map.cend(); ++it) {
    if (!(fields & it->first)) {
      cJSON *root = get_root(false, true, it->first);
      if (root) {
        cJSON *item = cJSON_GetObjectItem(root, it->second.c_str());
        if (item) {
          cJSON_DetachItemViaPointer(root, item);
          cJSON_Delete(item);
        }
      }
    }
  }

  if (!(fields & SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_CONTENT)) {
    cJSON *item = cJSON_GetObjectItem(get_properties_root(), content_available);
    if (item) {
      cJSON_DetachItemViaPointer(get_properties_root(), item);
      cJSON_Delete(item);
    }
  }

  remove_empty_sub_roots();
}

void device_json_config::remove_fields(unsigned _supla_int64_t fields) {
  for (auto it = field_map.cbegin(); it != field_map.cend(); ++it) {
    if (fields & it->first) {
      cJSON *root = get_root(false, true, it->first);
      if (root) {
        cJSON *item = cJSON_GetObjectItem(root, it->second.c_str());
        if (item) {
          cJSON_DetachItemViaPointer(root, item);
          cJSON_Delete(item);
        }
      }
    }
  }

  remove_empty_sub_roots();
}

void device_json_config::merge(supla_json_config *_dst) {
  device_json_config dst(_dst);

  map<unsigned _supla_int16_t, string> props_fields = {
      {1, content_available},
      {2, field_map.at(SUPLA_DEVICE_CONFIG_FIELD_MODBUS)}};

  map<cJSON *, map<unsigned _supla_int16_t, string>> map;

  for (auto it = field_map.cbegin(); it != field_map.cend(); ++it) {
    cJSON *root = get_root(false, true, it->first);
    if (root) {
      map[root][it->first] = it->second;
    }
  }

  for (auto rit = map.cbegin(); rit != map.cend(); ++rit) {
    for (auto fit = rit->second.cbegin(); fit != rit->second.cend(); ++fit) {
      supla_json_config::merge(rit->first, dst.get_root(true, true, fit->first),
                               rit->second, false);
    }
  }

  remove_empty_sub_roots();

  supla_json_config::merge(get_properties_root(), dst.get_properties_root(),
                           props_fields, false);
}
