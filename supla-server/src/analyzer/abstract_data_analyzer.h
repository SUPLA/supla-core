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

#include "device/extended_value/abstract_channel_extended_value.h"
#include "device/value/abstract_channel_value.h"
#include "jsonconfig/json_config.h"

#ifndef ABSTRACT_DATA_ANALYZER_H_
#define ABSTRACT_DATA_ANALYZER_H_

class supla_abstract_data_analyzer {
 private:
  int channel_id;

 public:
  supla_abstract_data_analyzer(void);
  explicit supla_abstract_data_analyzer(int channel_id);
  virtual ~supla_abstract_data_analyzer(void);
  void set_channel_id(int channel_id);
  int get_channel_id(void);

  virtual void add_sample(supla_abstract_channel_value *value,
                          supla_json_config *config) = 0;
  virtual void add_sample(
      int channel_flags, supla_json_config *config,
      supla_abstract_channel_extended_value *extended_value) = 0;

  virtual void reset(void) = 0;
  virtual bool is_any_data_for_logging_purpose(void) = 0;
  virtual supla_abstract_data_analyzer *copy(void) = 0;  // NOLINT
};

#endif /*ABSTRACT_DATA_ANALYZER_H_*/
