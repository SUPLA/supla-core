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

#include "electricity_analyzer.h"

#include "device/extended_value/channel_em_extended_value.h"
#include "jsonconfig/channel/electricity_meter_config.h"
#include "srpc/srpc.h"

supla_electricity_analyzer::supla_electricity_analyzer(void)
    : supla_abstract_data_analyzer() {
  aberration_phase1 = nullptr;
  aberration_phase2 = nullptr;
  aberration_phase3 = nullptr;
}

supla_electricity_analyzer::supla_electricity_analyzer(int channel_id)
    : supla_abstract_data_analyzer(channel_id) {
  aberration_phase1 = nullptr;
  aberration_phase2 = nullptr;
  aberration_phase3 = nullptr;
}

supla_electricity_analyzer::~supla_electricity_analyzer(void) {
  if (aberration_phase1) {
    delete aberration_phase1;
  }

  if (aberration_phase2) {
    delete aberration_phase2;
  }

  if (aberration_phase3) {
    delete aberration_phase3;
  }
}

supla_voltage_aberration_analyzer *
supla_electricity_analyzer::get_aberration_phase1(void) {
  return aberration_phase1;
}

supla_voltage_aberration_analyzer *
supla_electricity_analyzer::get_aberration_phase2(void) {
  return aberration_phase2;
}

supla_voltage_aberration_analyzer *
supla_electricity_analyzer::get_aberration_phase3(void) {
  return aberration_phase3;
}

void supla_electricity_analyzer::add_sample(
    double lower_voltage_threshold, double upper_voltage_threshold,
    double voltage, supla_voltage_aberration_analyzer **va) {
  if (*va == nullptr) {
    *va = new supla_voltage_aberration_analyzer();
    (*va)->set_lower_threshold(lower_voltage_threshold);
    (*va)->set_upper_threshold(upper_voltage_threshold);
  }

  (*va)->add_sample(voltage);
}

void supla_electricity_analyzer::add_sample(
    int channel_flags, supla_json_config *config,
    supla_channel_extended_value *extended_value) {
  double lower_voltage_threshold = 0;
  double upper_voltage_threshold = 0;
  supla_channel_em_extended_value *em_ev = nullptr;

  if (!config || !extended_value ||
      !(em_ev =
            dynamic_cast<supla_channel_em_extended_value *>(extended_value))) {
    return;
  }

  electricity_meter_config em_config(config);

  lower_voltage_threshold = em_config.get_lower_voltage_threshold();
  upper_voltage_threshold = em_config.get_upper_voltage_threshold();

  if (!lower_voltage_threshold && !upper_voltage_threshold) {
    return;
  }

  TElectricityMeter_ExtendedValue_V2 em_ev_raw = {};
  if (em_ev->get_raw_value(&em_ev_raw) &&
      (em_ev_raw.measured_values & EM_VAR_VOLTAGE) && em_ev_raw.m_count > 0) {
    if (!(channel_flags & SUPLA_CHANNEL_FLAG_PHASE1_UNSUPPORTED)) {
      add_sample(lower_voltage_threshold, upper_voltage_threshold,
                 em_ev_raw.m[0].voltage[0] / 100.00, &aberration_phase1);
    }

    if (!(channel_flags & SUPLA_CHANNEL_FLAG_PHASE2_UNSUPPORTED)) {
      add_sample(lower_voltage_threshold, upper_voltage_threshold,
                 em_ev_raw.m[0].voltage[1] / 100.00, &aberration_phase2);
    }

    if (!(channel_flags & SUPLA_CHANNEL_FLAG_PHASE3_UNSUPPORTED)) {
      add_sample(lower_voltage_threshold, upper_voltage_threshold,
                 em_ev_raw.m[0].voltage[2] / 100.00, &aberration_phase3);
    }
  }
}

void supla_electricity_analyzer::add_sample(supla_channel_value *value,
                                            supla_json_config *config) {}

bool supla_electricity_analyzer::is_any_data_for_logging_purpose(void) {
  if (aberration_phase1 && (aberration_phase1->get_below_count() > 0 ||
                            aberration_phase1->get_above_count() > 0)) {
    return true;
  }

  if (aberration_phase2 && (aberration_phase2->get_below_count() > 0 ||
                            aberration_phase2->get_above_count() > 0)) {
    return true;
  }
  if (aberration_phase3 && (aberration_phase3->get_below_count() > 0 ||
                            aberration_phase3->get_above_count() > 0)) {
    return true;
  }

  return false;
}

void supla_electricity_analyzer::reset(void) {
  if (aberration_phase1) {
    aberration_phase1->reset();
  }

  if (aberration_phase2) {
    aberration_phase2->reset();
  }

  if (aberration_phase3) {
    aberration_phase3->reset();
  }
}

supla_abstract_data_analyzer *supla_electricity_analyzer::copy(
    void) {  // NOLINT
  supla_electricity_analyzer *result =
      new supla_electricity_analyzer(get_channel_id());

  if (aberration_phase1) {
    result->aberration_phase1 = new supla_voltage_aberration_analyzer();
    *result->aberration_phase1 = *aberration_phase1;
  }

  if (aberration_phase2) {
    result->aberration_phase2 = new supla_voltage_aberration_analyzer();
    *result->aberration_phase2 = *aberration_phase2;
  }

  if (aberration_phase3) {
    result->aberration_phase3 = new supla_voltage_aberration_analyzer();
    *result->aberration_phase3 = *aberration_phase3;
  }

  return result;
}
