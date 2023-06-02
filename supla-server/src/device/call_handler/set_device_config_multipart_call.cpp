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

#include "set_device_config_multipart_call.h"

supla_set_device_multipart_call::supla_set_device_multipart_call(void)
    : supla_multipart_call(SUPLA_DS_CALL_SET_DEVICE_CONFIG, 3000) {}

supla_set_device_multipart_call::~supla_set_device_multipart_call() {}

void supla_set_device_multipart_call::part_push(TSDS_SetDeviceConfig *config) {
  if (config) {
    size_t data_size = sizeof(TSDS_SetDeviceConfig) -
                       SUPLA_DEVICE_CONFIG_MAXSIZE + config->ConfigSize;
    if (data_size > sizeof(TSDS_SetDeviceConfig)) {
      data_size = sizeof(TSDS_SetDeviceConfig);
    }
    supla_multipart_call::part_push((char *)config, data_size);
  }
}

bool supla_set_device_multipart_call::part_pop(TSDS_SetDeviceConfig *config) {
  if (!config) {
    return false;
  }

  memset(config, 0, sizeof(TSDS_SetDeviceConfig));

  size_t data_size = 0;
  bool result = false;
  char *data = supla_multipart_call::part_pop(&data_size);
  if (data) {
    if (data_size >=
            sizeof(TSDS_SetDeviceConfig) - SUPLA_DEVICE_CONFIG_MAXSIZE &&
        data_size <= sizeof(TSDS_SetDeviceConfig)) {
      memcpy(config, data, data_size);
      if (config->ConfigSize > SUPLA_DEVICE_CONFIG_MAXSIZE) {
        config->ConfigSize = SUPLA_DEVICE_CONFIG_MAXSIZE;
      }
      result = true;
    }
    delete[] data;
  }

  return result;
}
