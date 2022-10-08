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

#include "analyzer/voltage_analyzer.h"
#include "channeljsonconfig/electricity_meter_config.h"
#include "proto.h"

#ifndef VOLTAGE_ANALYZERS_H_
#define VOLTAGE_ANALYZERS_H_

class supla_voltage_analyzers {
 private:
  int channel_id;
  supla_voltage_analyzer *phase1;
  supla_voltage_analyzer *phase2;
  supla_voltage_analyzer *phase3;
  void add_sample(double lower_voltage_threshold,
                  double upper_voltage_threshold, double voltage,
                  supla_voltage_analyzer **va);

 public:
  supla_voltage_analyzers(void);
  supla_voltage_analyzers(const supla_voltage_analyzers &analyzers);  // NO LINT
  virtual ~supla_voltage_analyzers(void);
  void set_channel_id(int channel_id);
  int get_channel_id(void);
  supla_voltage_analyzer *get_phase1(void);
  supla_voltage_analyzer *get_phase2(void);
  supla_voltage_analyzer *get_phase3(void);
  void add_samples(int channel_flags, electricity_meter_config *config,
                   TSuplaChannelExtendedValue *extended_value);

  bool is_any_sample_over_threshold(void);
  void reset(void);

  supla_voltage_analyzers &operator=(const supla_voltage_analyzers &analyzers);
};

#endif /*VOLTAGE_ANALYZERS_H_*/
