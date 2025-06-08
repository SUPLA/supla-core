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
    : supla_simple_statiscics(), supla_abstract_data_analyzer() {
  any_last = NAN;
  last_sample_time = {};
  time_sum = 0;
  sample_sum = 0;
}

supla_general_purpose_measurement_analyzer::
    supla_general_purpose_measurement_analyzer(int channel_id)
    : supla_simple_statiscics(), supla_abstract_data_analyzer(channel_id) {
  any_last = NAN;
  last_sample_time = {};
  time_sum = 0;
  sample_sum = 0;
}

supla_general_purpose_measurement_analyzer::
    ~supla_general_purpose_measurement_analyzer(void) {}

void supla_general_purpose_measurement_analyzer::add_sample(
    supla_abstract_channel_value *value, supla_json_config *config) {
  supla_channel_general_purpose_measurement_value *gpm_val =
      dynamic_cast<supla_channel_general_purpose_measurement_value *>(value);
  if (gpm_val) {
    general_purpose_base_config gp_config(config);
    if (gp_config.keep_history()) {
      add_sample(gpm_val->get_value());
    }
  }
}

void supla_general_purpose_measurement_analyzer::add_sample(double sample) {
  supla_simple_statiscics::add_sample(sample);
  auto now = std::chrono::steady_clock::now();

  if (!isnan(any_last)) {
    long long time_diff =
        std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::steady_clock::now() - last_sample_time)
            .count();
    time_sum += time_diff;
    sample_sum += any_last * time_diff;
  }

  any_last = sample;
  last_sample_time = now;
}

double supla_general_purpose_measurement_analyzer::get_time_weighted_avg(void) {
  if (last_sample_time != std::chrono::steady_clock::time_point()) {
    long long time_sum = this->time_sum;
    double sample_sum = this->sample_sum;

    if (!isnan(any_last)) {
      long long time_diff =
          std::chrono::duration_cast<std::chrono::seconds>(
              std::chrono::steady_clock::now() - last_sample_time)
              .count();
      time_sum += time_diff;
      sample_sum += any_last * time_diff;
    }

    if (time_sum > 0) {
      return sample_sum / time_sum;
    }
  }

  return NAN;
}

double supla_general_purpose_measurement_analyzer::get_any_last(void) {
  return any_last;
}

void supla_general_purpose_measurement_analyzer::add_sample(
    int channel_flags, supla_json_config *config,
    supla_abstract_channel_extended_value *extended_value) {}

bool supla_general_purpose_measurement_analyzer::
    is_any_data_for_logging_purpose(void) {
  return !isnan(get_time_weighted_avg());
}

void supla_general_purpose_measurement_analyzer::reset(void) {
  supla_simple_statiscics::reset();
  any_last = NAN;
  last_sample_time = {};
  time_sum = 0;
  sample_sum = 0;
}

supla_abstract_data_analyzer *supla_general_purpose_measurement_analyzer::copy(
    void) {  // NOLINT
  supla_general_purpose_measurement_analyzer *result =
      new supla_general_purpose_measurement_analyzer();
  *result = *this;
  return result;
}
