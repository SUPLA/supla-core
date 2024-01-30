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

#include "general_purpose_measurement_analyzer.h"

#include <math.h>

#include "device/value/channel_general_purpose_measurement_value.h"
#include "jsonconfig/channel/general_purpose_base_config.h"

supla_general_purpose_measurement_analyzer::
    supla_general_purpose_measurement_analyzer(void)
    : supla_simple_statiscics(), supla_abstract_data_analyzer() {}

supla_general_purpose_measurement_analyzer::
    supla_general_purpose_measurement_analyzer(int channel_id)
    : supla_simple_statiscics(), supla_abstract_data_analyzer(channel_id) {}

supla_general_purpose_measurement_analyzer::
    ~supla_general_purpose_measurement_analyzer(void) {}

void supla_general_purpose_measurement_analyzer::add_sample(
    supla_channel_value *value, supla_json_config *config) {
  supla_channel_general_purpose_measurement_value *gpm_val =
      dynamic_cast<supla_channel_general_purpose_measurement_value *>(value);
  if (gpm_val && gpm_val->get_value() != NAN) {
    general_purpose_base_config gp_config(config);
    if (gp_config.keep_history()) {
      supla_simple_statiscics::add_sample(gpm_val->get_value());
    }
  }
}

void supla_general_purpose_measurement_analyzer::add_sample(
    int channel_flags, supla_json_config *config,
    supla_channel_extended_value *extended_value) {}

bool supla_general_purpose_measurement_analyzer::
    is_any_data_for_logging_purpose(void) {
  return supla_simple_statiscics::get_sample_count() > 0;
}

void supla_general_purpose_measurement_analyzer::reset(void) {
  supla_simple_statiscics::reset();
}

supla_abstract_data_analyzer *supla_general_purpose_measurement_analyzer::copy(
    void) {  // NOLINT
  supla_general_purpose_measurement_analyzer *result =
      new supla_general_purpose_measurement_analyzer();
  *result = *this;
  return result;
}
