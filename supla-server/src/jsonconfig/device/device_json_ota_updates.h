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

#ifndef DEVICE_JSON_OTA_UPDATES_H_
#define DEVICE_JSON_OTA_UPDATES_H_

#include <map>
#include <string>

#include "jsonconfig/json_config.h"
#include "proto.h"

class device_json_ota_updates : public supla_json_config {
 private:
  static const std::map<unsigned _supla_int16_t, std::string> field_map;
  cJSON *create_status_object(std::string status);
  void apply_on_properties_root(cJSON *ota);

 protected:
  virtual __time_t get_timestamp(void);

 public:
  device_json_ota_updates(void);
  explicit device_json_ota_updates(supla_json_config *root);
  virtual ~device_json_ota_updates(void);

  void set_checking(void);
  void set_not_available(void);
  void set_available(std::string soft_ver, std::string changelog_url);
  void set_error(void);

  virtual void merge(supla_json_config *dst);
};

#endif /* DEVICE_JSON_OTA_UPDATES_H_ */
