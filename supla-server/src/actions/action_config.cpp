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

#include "action_config.h"

#include <stdlib.h>
#include <strings.h>
#include <time.h>

#include "actions/action_hvac_setpoint_temperature.h"
#include "actions/action_hvac_setpoint_temperatures.h"
#include "actions/action_rgbw_parameters.h"
#include "actions/action_rs_parameters.h"
#include "json/cJSON.h"
#include "tools.h"

supla_action_config::supla_action_config(void) : abstract_action_config() {
  action_id = 0;
  subject_type = stUnknown;
  subject_id = 0;
  source_device_id = 0;
  source_channel_id = 0;
  parameters = nullptr;
}

supla_action_config::supla_action_config(supla_action_config *src) {
  action_id = src->action_id;
  subject_type = src->subject_type;
  subject_id = src->subject_id;
  source_device_id = src->source_device_id;
  source_channel_id = src->source_channel_id;
  parameters = src->parameters ? src->parameters->copy() : nullptr;
}

supla_action_config::supla_action_config(const supla_action_config &src) {
  action_id = src.action_id;
  subject_type = src.subject_type;
  subject_id = src.subject_id;
  source_device_id = src.source_device_id;
  source_channel_id = src.source_channel_id;
  parameters = src.parameters ? src.parameters->copy() : nullptr;
}

supla_action_config::~supla_action_config(void) {
  if (parameters) {
    delete parameters;
  }
}

int supla_action_config::get_action_id(void) { return action_id; }

void supla_action_config::set_action_id(int action_id) {
  this->action_id = action_id;
}

_subjectType_e supla_action_config::get_subject_type(void) {
  return subject_type;
}

void supla_action_config::set_subject_type(_subjectType_e subject_type) {
  this->subject_type = subject_type;
}

int supla_action_config::get_subject_id(void) { return subject_id; }

void supla_action_config::set_subject_id(int subject_id) {
  this->subject_id = subject_id;
}

int supla_action_config::get_source_device_id(void) { return source_device_id; }

void supla_action_config::set_source_device_id(int source_device_id) {
  this->source_device_id = source_device_id;
}

int supla_action_config::get_source_channel_id(void) {
  return source_channel_id;
}

void supla_action_config::set_source_channel_id(int source_channel_id) {
  this->source_channel_id = source_channel_id;
}

supla_abstract_action_parameters *supla_action_config::get_parameters(void) {
  return parameters ? parameters->copy() : nullptr;
}

void supla_action_config::set_parameters(
    supla_abstract_action_parameters *parameters) {
  if (this->parameters) {
    delete this->parameters;
  }

  this->parameters = parameters ? parameters->copy() : nullptr;
}

void supla_action_config::apply_json_params(const char *params) {
  cJSON *root = cJSON_Parse(params);
  if (!root) {
    return;
  }

  cJSON *item = cJSON_GetObjectItem(root, "percentage");

  if (item) {
    supla_action_rs_parameters p;
    if (cJSON_IsNumber(item)) {
      if (item->valuedouble < 0) {
        p.set_percentage(0);
      } else if (item->valuedouble > 100) {
        p.set_percentage(100);
      } else {
        p.set_percentage(item->valuedouble);
      }
      set_parameters(&p);
    }
  }

  item = cJSON_GetObjectItem(root, "sourceChannelId");

  if (item && cJSON_IsNumber(item)) {
    set_source_channel_id(item->valuedouble);
  }

  item = cJSON_GetObjectItem(root, "sourceDeviceId");

  if (item && cJSON_IsNumber(item)) {
    set_source_device_id(item->valuedouble);
  }

  {
    supla_action_rgbw_parameters rgbw;
    bool rgbw_changed = false;

    item = cJSON_GetObjectItem(root, "brightness");
    if (item && cJSON_IsNumber(item)) {
      if (item->valuedouble < 0) {
        rgbw.set_brightness(0);
      } else if (item->valuedouble > 100) {
        rgbw.set_brightness(100);
      } else {
        rgbw.set_brightness(item->valuedouble);
      }

      rgbw_changed = true;
    }

    item = cJSON_GetObjectItem(root, "color_brightness");
    if (item && cJSON_IsNumber(item)) {
      if (item->valuedouble < 0) {
        rgbw.set_color_brightness(0);
      } else if (item->valuedouble > 100) {
        rgbw.set_color_brightness(100);
      } else {
        rgbw.set_color_brightness(item->valuedouble);
      }
      rgbw_changed = true;
    }

    item = cJSON_GetObjectItem(root, "hue");
    if (item) {
      if (cJSON_IsNumber(item)) {
        rgbw.set_color(st_hue2rgb(item->valuedouble));
        rgbw_changed = true;
      } else if (cJSON_IsString(item)) {
        if (strncasecmp(cJSON_GetStringValue(item), "random", 255) == 0) {
          rgbw.set_random_color(true);
          rgbw_changed = true;
        } else if (strncasecmp(cJSON_GetStringValue(item), "white", 255) == 0) {
          rgbw.set_color(0xFFFFFF);
          rgbw_changed = true;
        }
      }
    }

    if (rgbw_changed) {
      set_parameters(&rgbw);
    }
  }

  item = cJSON_GetObjectItem(root, "temperature");

  if (item && cJSON_IsNumber(item)) {
    supla_action_hvac_setpoint_temperature t(item->valueint);
    set_parameters(&t);
  }

  {
    supla_action_hvac_setpoint_temperatures hvac_temperatures;

    item = cJSON_GetObjectItem(root, "temperatureHeat");

    if ((item && cJSON_IsNumber(item))) {
      hvac_temperatures.set_heating_temperature(item->valueint);
    }

    item = cJSON_GetObjectItem(root, "temperatureCool");

    if ((item && cJSON_IsNumber(item))) {
      hvac_temperatures.set_cooling_temperature(item->valueint);
    }

    if (hvac_temperatures.is_any_temperature_set()) {
      set_parameters(&hvac_temperatures);
    }
  }

  cJSON_Delete(root);
}

bool supla_action_config::operator==(const supla_action_config &config) const {
  return action_id == config.action_id && subject_type == config.subject_type &&
         subject_id == config.subject_id &&
         source_device_id == config.source_device_id &&
         source_channel_id == config.source_channel_id &&
         (parameters == config.parameters ||
          (parameters != nullptr && config.parameters != nullptr &&
           parameters->equal(config.parameters)));
}

supla_action_config &supla_action_config::operator=(
    const supla_action_config &config) {
  action_id = config.action_id;
  subject_type = config.subject_type;
  subject_id = config.subject_id;
  source_device_id = config.source_device_id;
  source_channel_id = config.source_channel_id;

  set_parameters(config.parameters);
  return *this;
}
