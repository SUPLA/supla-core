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

supla_channel_rgbw_value::supla_channel_rgbw_value(
    char native_value[SUPLA_CHANNELVALUE_SIZE])
    : supla_channel_value(native_value) {}

supla_channel_rgbw_value::supla_channel_rgbw_value(TRGBW_Value *rgbw)
    : supla_channel_value() {
  set_rgbw(rgbw);
}

void supla_channel_rgbw_value::set_rgbw(TRGBW_Value *rgbw) {
  if (rgbw) {
    memcpy(native_value, rgbw, sizeof(TRGBW_Value));
  }
}

void supla_channel_rgbw_value::get_rgbw(TRGBW_Value *rgbw) {
  if (rgbw) {
    memcpy(rgbw, native_value, sizeof(TRGBW_Value));
  }
}

unsigned int supla_channel_rgbw_value::get_color(void) {
  unsigned int color = 0;
  TRGBW_Value *rgbw = (TRGBW_Value *)native_value;
  color = ((unsigned int)rgbw->R << 16) & 0x00FF0000;
  color |= ((unsigned int)rgbw->G << 8) & 0x0000FF00;
  color |= ((unsigned int)rgbw->B) & 0x000000FF;
  return color;
}

void supla_channel_rgbw_value::set_color(unsigned int color) {
  TRGBW_Value *rgbw = (TRGBW_Value *)native_value;
  rgbw->B = color & 0xFF;
  color >>= 8;
  rgbw->G = color & 0xFF;
  color >>= 8;
  rgbw->R = color & 0xFF;
}

char supla_channel_rgbw_value::get_brightness(void) {
  TRGBW_Value *rgbw = (TRGBW_Value *)native_value;
  return rgbw->brightness;
}

void supla_channel_rgbw_value::set_brightness(char brightness) {
  TRGBW_Value *rgbw = (TRGBW_Value *)native_value;
  rgbw->brightness = brightness;
}

char supla_channel_rgbw_value::get_color_brightness(void) {
  TRGBW_Value *rgbw = (TRGBW_Value *)native_value;
  return rgbw->colorBrightness;
}

void supla_channel_rgbw_value::set_color_brightness(char brightness) {
  TRGBW_Value *rgbw = (TRGBW_Value *)native_value;
  rgbw->colorBrightness = brightness;
}
