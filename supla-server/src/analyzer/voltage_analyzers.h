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
#include "analyzer/voltage_analyzer.h"

#ifndef VOLTAGE_ANALYZERS_H_
#define VOLTAGE_ANALYZERS_H_

class supla_voltage_analyzers : public supla_abstract_data_analyzer {
 private:
  supla_voltage_analyzer *phase1;
  supla_voltage_analyzer *phase2;
  supla_voltage_analyzer *phase3;
  void add_sample(double lower_voltage_threshold,
                  double upper_voltage_threshold, double voltage,
                  supla_voltage_analyzer **va);

 public:
  supla_voltage_analyzers(void);
  explicit supla_voltage_analyzers(int channel_id);
  virtual ~supla_voltage_analyzers(void);

  supla_voltage_analyzer *get_phase1(void);
  supla_voltage_analyzer *get_phase2(void);
  supla_voltage_analyzer *get_phase3(void);

  virtual void add_samples(supla_channel_value *value);
  virtual void add_samples(int channel_flags, supla_json_config *config,
                           supla_channel_extended_value *extended_value);

  virtual void reset(void);
  virtual bool is_any_data_for_logging_purpose(void);
  virtual supla_abstract_data_analyzer *copy(void);  // NOLINT
};

#endif /*VOLTAGE_ANALYZERS_H_*/
