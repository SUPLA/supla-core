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

#ifndef CHANNEL_GENERAL_PURPOSE_TEXT_EXTENDED_VALUE_H_
#define CHANNEL_GENERAL_PURPOSE_TEXT_EXTENDED_VALUE_H_

#include <string>

#include "device/extended_value/abstract_channel_extended_value.h"

class supla_channel_general_purpose_text_extended_value
    : public supla_abstract_channel_extended_value {
 public:
  supla_channel_general_purpose_text_extended_value();
  explicit supla_channel_general_purpose_text_extended_value(
      const TSuplaChannelExtendedValue *value);
  virtual ~supla_channel_general_purpose_text_extended_value(void);
  virtual supla_abstract_channel_extended_value *copy(void);  // NOLINT

  bool get_vbt_text_value(_vbt_var_name_e var_name,
                          std::string *value) override;
  nlohmann::json get_template_data(void) override;
  std::string get_text(void);
  static bool is_ev_type_supported(char type);
};

#endif /*CHANNEL_GENERAL_PURPOSE_TEXT_EXTENDED_VALUE_H_*/
