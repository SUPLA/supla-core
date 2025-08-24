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

#ifndef DEVICE_JSON_CFG_MODE_PASSWORD_H_
#define DEVICE_JSON_CFG_MODE_PASSWORD_H_

#include <map>
#include <string>

#include "jsonconfig/device/device_json_async_result.h"
#include "proto.h"

class device_json_cfg_mode_password : public device_json_async_result {
 private:
  static const std::map<unsigned _supla_int16_t, std::string> field_map;

 protected:
  virtual const std::map<unsigned _supla_int16_t, std::string> &get_field_map(
      void);
  virtual std::string get_status_root_key(void);

 public:
  device_json_cfg_mode_password(void);
  explicit device_json_cfg_mode_password(supla_json_config *root);
  virtual ~device_json_cfg_mode_password(void);

  void set_true(void);
  void set_false(void);
  void set_waiting(void);
  void set_unauthorized(void);
  void set_not_supported(void);
  virtual bool set_calcfg_result(TDS_DeviceCalCfgResult *result);
};

#endif /* DEVICE_JSON_CFG_MODE_PASSWORD_H_ */
