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

#ifndef DEVICE_JSON_ASYNC_RESULT_H_
#define DEVICE_JSON_ASYNC_RESULT_H_

#include <map>
#include <string>

#include "jsonconfig/json_config.h"
#include "proto.h"

class device_json_async_result : public supla_json_config {
 private:
  static const std::string status_key_str;

 protected:
  cJSON *create_timestamp_object(void);
  cJSON *create_status_object(std::string status);
  void apply_on_properties_root(cJSON *ota);
  virtual __time_t get_timestamp(void);
  virtual const std::map<unsigned _supla_int16_t, std::string> &get_field_map(
      void) = 0;
  virtual std::string get_status_root_key(void) = 0;

 public:
  device_json_async_result(void);
  explicit device_json_async_result(supla_json_config *root);
  virtual ~device_json_async_result(void);

  virtual void merge(supla_json_config *dst);
  virtual bool set_calcfg_result(TDS_DeviceCalCfgResult *result) = 0;
};

#endif /* DEVICE_JSON_ASYNC_RESULT_H_ */
