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

#include "action_rgbw_parameters.h"

#include <stdlib.h>

#include "tools.h"

supla_action_rgbw_parameters::supla_action_rgbw_parameters(void)
    : supla_abstract_action_parameters() {
  rgbw = {};
}

supla_action_rgbw_parameters::supla_action_rgbw_parameters(
    const TAction_RGBW_Parameters &rgbw)
    : supla_abstract_action_parameters() {
  this->rgbw = rgbw;
}

supla_action_rgbw_parameters::supla_action_rgbw_parameters(
    const TAction_RGBW_Parameters *rgbw) {
  this->rgbw = {};
  if (rgbw) {
    this->rgbw = *rgbw;
  }
}

supla_action_rgbw_parameters::~supla_action_rgbw_parameters(void) {}

bool supla_action_rgbw_parameters::equal(
    supla_abstract_action_parameters *params) const {
  supla_action_rgbw_parameters *p =
      dynamic_cast<supla_action_rgbw_parameters *>(params);

  return p && rgbw.Brightness == p->rgbw.Brightness &&
         rgbw.Color == p->rgbw.Color &&
         rgbw.ColorBrightness == p->rgbw.ColorBrightness &&
         rgbw.ColorRandom == p->rgbw.ColorRandom && rgbw.OnOff == p->rgbw.OnOff;
}

supla_abstract_action_parameters *supla_action_rgbw_parameters::copy(  // NOLINT
    void) {                                                            // NOLINT
  return new supla_action_rgbw_parameters(rgbw);
}

const TAction_RGBW_Parameters supla_action_rgbw_parameters::get_rgbw(void) {
  TAction_RGBW_Parameters result = rgbw;
  if (result.ColorRandom) {
    while (!result.Color) {
      struct timeval time = {};
      gettimeofday(&time, nullptr);
      unsigned int seed = time.tv_sec + time.tv_usec;
      result.Color = st_hue2rgb(rand_r(&seed) % 360);
    }
  }
  return result;
}

void supla_action_rgbw_parameters::set_brightness(char brightness) {
  rgbw.Brightness = brightness;
}

void supla_action_rgbw_parameters::set_color_brightness(char brightness) {
  rgbw.ColorBrightness = brightness;
}

void supla_action_rgbw_parameters::set_color(unsigned int color) {
  rgbw.Color = color;
}

void supla_action_rgbw_parameters::set_random_color(bool random) {
  rgbw.ColorRandom = random ? 1 : 0;
}
