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

#include "channel_rgbw_value.h"

#include <string.h>

#include <cstdio>
#include <string>

using std::string;

supla_channel_rgbw_value::supla_channel_rgbw_value(int func)
    : supla_abstract_channel_value() {
  this->func = func;
}

supla_channel_rgbw_value::supla_channel_rgbw_value(
    int func, const char raw_value[SUPLA_CHANNELVALUE_SIZE])
    : supla_abstract_channel_value(raw_value) {
  this->func = func;
}

supla_channel_rgbw_value::supla_channel_rgbw_value(int func, TRGBW_Value *rgbw)
    : supla_abstract_channel_value() {
  this->func = func;
  set_rgbw(rgbw);
}

supla_abstract_channel_value *supla_channel_rgbw_value::copy(  // NOLINT
    void) const {                                              // NOLINT
  return new supla_channel_rgbw_value(func, raw_value);
}

void supla_channel_rgbw_value::set_rgbw(TRGBW_Value *rgbw) {
  if (rgbw) {
    memcpy(raw_value, rgbw, sizeof(TRGBW_Value));
  }
}

void supla_channel_rgbw_value::get_rgbw(TRGBW_Value *rgbw) {
  if (rgbw) {
    memcpy(rgbw, raw_value, sizeof(TRGBW_Value));
  }
}

void supla_channel_rgbw_value::get_rgbw(int *color, char *color_brightness,
                                        char *brightness,
                                        char *white_temperature) {
  if (color) {
    *color = get_color();
  }

  if (color_brightness) {
    *color_brightness = get_color_brightness();
  }

  if (brightness) {
    *brightness = get_brightness();
  }

  if (white_temperature) {
    *white_temperature = get_white_temperature();
  }
}

unsigned int supla_channel_rgbw_value::get_color(void) {
  unsigned int color = 0;
  TRGBW_Value *rgbw = (TRGBW_Value *)raw_value;
  color = ((unsigned int)rgbw->R << 16) & 0x00FF0000;
  color |= ((unsigned int)rgbw->G << 8) & 0x0000FF00;
  color |= ((unsigned int)rgbw->B) & 0x000000FF;
  return color;
}

string supla_channel_rgbw_value::get_color_hex(void) {
  TRGBW_Value *rgbw = (TRGBW_Value *)raw_value;
  char color_hex[9] = {};
  snprintf(color_hex, sizeof(color_hex), "0x%02X%02X%02X", rgbw->R, rgbw->G,
           rgbw->B);

  return color_hex;
}

void supla_channel_rgbw_value::set_color(unsigned int color) {
  TRGBW_Value *rgbw = (TRGBW_Value *)raw_value;
  rgbw->B = color & 0xFF;
  color >>= 8;
  rgbw->G = color & 0xFF;
  color >>= 8;
  rgbw->R = color & 0xFF;
}

char supla_channel_rgbw_value::get_brightness(void) {
  TRGBW_Value *rgbw = (TRGBW_Value *)raw_value;
  return rgbw->brightness;
}

void supla_channel_rgbw_value::set_brightness(char brightness) {
  TRGBW_Value *rgbw = (TRGBW_Value *)raw_value;
  rgbw->brightness = brightness;
}

char supla_channel_rgbw_value::get_color_brightness(void) {
  TRGBW_Value *rgbw = (TRGBW_Value *)raw_value;
  return rgbw->colorBrightness;
}

void supla_channel_rgbw_value::set_color_brightness(char brightness) {
  TRGBW_Value *rgbw = (TRGBW_Value *)raw_value;
  rgbw->colorBrightness = brightness;
}

char supla_channel_rgbw_value::get_command(void) {
  TRGBW_Value *rgbw = (TRGBW_Value *)raw_value;
  return rgbw->command;
}

void supla_channel_rgbw_value::set_command(char command) {
  TRGBW_Value *rgbw = (TRGBW_Value *)raw_value;
  rgbw->command = command;
}

char supla_channel_rgbw_value::get_white_temperature(void) {
  TRGBW_Value *rgbw = (TRGBW_Value *)raw_value;
  return rgbw->whiteTemperature;
}

void supla_channel_rgbw_value::set_white_temperature(char wt) {
  TRGBW_Value *rgbw = (TRGBW_Value *)raw_value;
  rgbw->whiteTemperature = wt;
}

// static
bool supla_channel_rgbw_value::is_function_supported(int func) {
  switch (func) {
    case SUPLA_CHANNELFNC_DIMMER:
    case SUPLA_CHANNELFNC_RGBLIGHTING:
    case SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING:
    case SUPLA_CHANNELFNC_DIMMER_CCT_AND_RGB:
    case SUPLA_CHANNELFNC_DIMMER_CCT:
      return true;
  }

  return false;
}

bool supla_channel_rgbw_value::is_on(void) {
  bool result = false;

  switch (func) {
    case SUPLA_CHANNELFNC_DIMMER:
    case SUPLA_CHANNELFNC_DIMMER_CCT:
      result = get_brightness() > 0;
      break;
    case SUPLA_CHANNELFNC_RGBLIGHTING:
      result = get_color_brightness() > 0;
      break;
    case SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING:
    case SUPLA_CHANNELFNC_DIMMER_CCT_AND_RGB:
      result = get_brightness() > 0 || get_color_brightness() > 0;
      break;
  }

  return result;
}

nlohmann::json supla_channel_rgbw_value::get_template_data(void) {
  nlohmann::json result = supla_abstract_channel_value::get_template_data();

  switch (func) {
    case SUPLA_CHANNELFNC_RGBLIGHTING:
    case SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING:
    case SUPLA_CHANNELFNC_DIMMER_CCT_AND_RGB:
      result["color"] = get_color_hex();
      result["color_brightness"] = get_color_brightness();
      break;
  }

  switch (func) {
    case SUPLA_CHANNELFNC_DIMMER:
    case SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING:
    case SUPLA_CHANNELFNC_DIMMER_CCT_AND_RGB:
    case SUPLA_CHANNELFNC_DIMMER_CCT:
      result["brightness"] = get_brightness();
      break;
  }

  switch (func) {
    case SUPLA_CHANNELFNC_DIMMER_CCT_AND_RGB:
    case SUPLA_CHANNELFNC_DIMMER_CCT:
      result["white_temperature"] = get_white_temperature();
      break;
  }

  result["on"] = is_on();

  return result;
}

bool supla_channel_rgbw_value::get_vbt_value(_vbt_var_name_e var_name,
                                             double *value) {
  switch (var_name) {
    case var_name_color:
      *value = get_color();
      break;
    case var_name_color_brightness:
      *value = get_color_brightness();
      break;
    case var_name_brightness:
      *value = get_brightness();
      break;
    default:
      return false;
  }
  return true;
}
