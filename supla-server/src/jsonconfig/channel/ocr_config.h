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

#ifndef OCR_CONFIG_H_
#define OCR_CONFIG_H_

#include <map>
#include <string>

#include "jsonconfig/channel/general_purpose_base_config.h"
#include "proto.h"

class ocr_config : public supla_json_config {
 private:
  static const std::map<unsigned _supla_int16_t, std::string> field_map;
  cJSON *get_ocr_root(cJSON *root, bool force);
  std::string lighting_mode_to_string(unsigned char mode);
  unsigned char string_lighting_mode(std::string mode);

 protected:
  virtual cJSON *_get_user_root(void);
  virtual cJSON *_get_properties_root(bool force = true);

 public:
  explicit ocr_config(supla_json_config *root);
  ocr_config(void);
  virtual void merge(supla_json_config *dst);
  void set_config(TChannelConfig_OCR *config);
  bool get_config(TChannelConfig_OCR *config);
};

#endif /* OCR_CONFIG_H_ */
