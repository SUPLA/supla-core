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

#include "analyzer/abstract_data_analyzer.h"
#include "analyzer/simple_statistics.h"
#include "analyzer/voltage_aberrarton_analyzer.h"

#ifndef VOLTAGE_ANALYZERS_H_
#define VOLTAGE_ANALYZERS_H_

class supla_electricity_analyzer : public supla_abstract_data_analyzer {
 private:
  supla_voltage_aberration_analyzer *aberration_phase1;
  supla_voltage_aberration_analyzer *aberration_phase2;
  supla_voltage_aberration_analyzer *aberration_phase3;

  supla_simple_statiscics *voltage_phase1;
  supla_simple_statiscics *voltage_phase2;
  supla_simple_statiscics *voltage_phase3;

  supla_simple_statiscics *current_phase1;
  supla_simple_statiscics *current_phase2;
  supla_simple_statiscics *current_phase3;

  supla_simple_statiscics *power_active_phase1;
  supla_simple_statiscics *power_active_phase2;
  supla_simple_statiscics *power_active_phase3;

  void add_sample(double lower_voltage_threshold,
                  double upper_voltage_threshold, double voltage,
                  supla_voltage_aberration_analyzer **va);

  void add_sample(bool logger_enabled, double sample,
                  supla_simple_statiscics **sa);

 public:
  supla_electricity_analyzer(void);
  explicit supla_electricity_analyzer(int channel_id);
  virtual ~supla_electricity_analyzer(void);

  supla_voltage_aberration_analyzer *get_aberration_phase1(void);
  supla_voltage_aberration_analyzer *get_aberration_phase2(void);
  supla_voltage_aberration_analyzer *get_aberration_phase3(void);

  supla_simple_statiscics *get_voltage_phase1(void);
  supla_simple_statiscics *get_voltage_phase2(void);
  supla_simple_statiscics *get_voltage_phase3(void);

  supla_simple_statiscics *get_current_phase1(void);
  supla_simple_statiscics *get_current_phase2(void);
  supla_simple_statiscics *get_current_phase3(void);

  supla_simple_statiscics *get_power_active_phase1(void);
  supla_simple_statiscics *get_power_active_phase2(void);
  supla_simple_statiscics *get_power_active_phase3(void);

  virtual void add_sample(supla_abstract_channel_value *value,
                          supla_json_config *config);
  virtual void add_sample(
      int channel_flags, supla_json_config *config,
      supla_abstract_channel_extended_value *extended_value);

  virtual void reset_aberrations(void);
  virtual void reset_voltage(void);
  virtual void reset_current(void);
  virtual void reset_power_active(void);
  virtual void reset(void);

  virtual bool is_any_aberration_for_logging_purpose(void);
  virtual bool is_any_voltage_for_logging_purpose(void);
  virtual bool is_any_current_for_logging_purpose(void);
  virtual bool is_any_power_active_for_logging_purpose(void);
  virtual bool is_any_data_for_logging_purpose(void);
  virtual supla_abstract_data_analyzer *copy(void);  // NOLINT
};

#endif /*VOLTAGE_ANALYZERS_H_*/
