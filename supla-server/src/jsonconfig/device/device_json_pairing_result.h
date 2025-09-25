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

#ifndef DEVICE_JSON_PAIRING_RESULT_H_
#define DEVICE_JSON_PAIRING_RESULT_H_

#include <sys/types.h>

#include <ctime>

#include "json/json_helper.h"
#include "proto.h"

class device_json_pairing_result {
 private:
  void add_time_object(cJSON *root, const char *key, int inc_seconds);

 protected:
  virtual std::time_t get_time(void);

 public:
  device_json_pairing_result(void);
  virtual ~device_json_pairing_result(void);
  char *calcfg_result_to_json(TDS_DeviceCalCfgResult *result);
  char *pairing_request_sent(void);
};

#endif /* DEVICE_JSON_PAIRING_RESULT_H_ */
