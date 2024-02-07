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

  voltage_phase1 = nullptr;
  voltage_phase2 = nullptr;
  voltage_phase3 = nullptr;

  current_phase1 = nullptr;
  current_phase2 = nullptr;
  current_phase3 = nullptr;

  power_active_phase1 = nullptr;
  power_active_phase2 = nullptr;
  power_active_phase3 = nullptr;
}

supla_electricity_analyzer::supla_electricity_analyzer(int channel_id)
    : supla_abstract_data_analyzer(channel_id) {
  aberration_phase1 = nullptr;
  aberration_phase2 = nullptr;
  aberration_phase3 = nullptr;

  voltage_phase1 = nullptr;
  voltage_phase2 = nullptr;
  voltage_phase3 = nullptr;

  current_phase1 = nullptr;
  current_phase2 = nullptr;
  current_phase3 = nullptr;

  power_active_phase1 = nullptr;
  power_active_phase2 = nullptr;
  power_active_phase3 = nullptr;
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

  if (voltage_phase1) {
    delete voltage_phase1;
  }

  if (voltage_phase2) {
    delete voltage_phase2;
  }

  if (voltage_phase3) {
    delete voltage_phase3;
  }

  if (current_phase1) {
    delete current_phase1;
  }

  if (current_phase2) {
    delete current_phase2;
  }

  if (current_phase3) {
    delete current_phase3;
  }

  if (power_active_phase1) {
    delete power_active_phase1;
  }

  if (power_active_phase2) {
    delete power_active_phase2;
  }

  if (power_active_phase3) {
    delete power_active_phase3;
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

supla_simple_statiscics *supla_electricity_analyzer::get_voltage_phase1(void) {
  return voltage_phase1;
}

supla_simple_statiscics *supla_electricity_analyzer::get_voltage_phase2(void) {
  return voltage_phase2;
}

supla_simple_statiscics *supla_electricity_analyzer::get_voltage_phase3(void) {
  return voltage_phase3;
}

supla_simple_statiscics *supla_electricity_analyzer::get_current_phase1(void) {
  return current_phase1;
}

supla_simple_statiscics *supla_electricity_analyzer::get_current_phase2(void) {
  return current_phase2;
}

supla_simple_statiscics *supla_electricity_analyzer::get_current_phase3(void) {
  return current_phase3;
}

supla_simple_statiscics *supla_electricity_analyzer::get_power_active_phase1(
    void) {
  return power_active_phase1;
}

supla_simple_statiscics *supla_electricity_analyzer::get_power_active_phase2(
    void) {
  return power_active_phase2;
}

supla_simple_statiscics *supla_electricity_analyzer::get_power_active_phase3(
    void) {
  return power_active_phase3;
}

void supla_electricity_analyzer::add_sample(
    double lower_voltage_threshold, double upper_voltage_threshold,
    double voltage, supla_voltage_aberration_analyzer **va) {
  if (lower_voltage_threshold || upper_voltage_threshold) {
    if (*va == nullptr) {
      *va = new supla_voltage_aberration_analyzer();
      (*va)->set_lower_threshold(lower_voltage_threshold);
      (*va)->set_upper_threshold(upper_voltage_threshold);
    }

    (*va)->add_sample(voltage);
  }
}

void supla_electricity_analyzer::add_sample(bool logger_enabled, double sample,
                                            supla_simple_statiscics **sa) {
  if (logger_enabled) {
    if (*sa == nullptr) {
      *sa = new supla_simple_statiscics();
    }

    (*sa)->add_sample(sample);
  }
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

  bool voltage_logger_enabled = em_config.is_voltage_logger_enabled();
  bool current_logger_enabled = em_config.is_current_logger_enabled();
  bool power_logger_enabled = em_config.is_power_active_logger_enabled();

  TElectricityMeter_ExtendedValue_V2 em_ev_raw = {};
  if (em_ev->get_raw_value(&em_ev_raw) && em_ev_raw.m_count > 0) {
    if (!(channel_flags & SUPLA_CHANNEL_FLAG_PHASE1_UNSUPPORTED)) {
      if (em_ev_raw.measured_values & EM_VAR_VOLTAGE) {
        add_sample(lower_voltage_threshold, upper_voltage_threshold,
                   em_ev->get_voltage(1), &aberration_phase1);

        add_sample(voltage_logger_enabled, em_ev->get_voltage(1),
                   &voltage_phase1);
      }

      if ((em_ev_raw.measured_values & EM_VAR_CURRENT) ||
          (em_ev_raw.measured_values & EM_VAR_CURRENT_OVER_65A)) {
        add_sample(current_logger_enabled, em_ev->get_current(1),
                   &current_phase1);
      }

      if (em_ev_raw.measured_values & EM_VAR_POWER_ACTIVE) {
        add_sample(power_logger_enabled, em_ev->get_power_active(1),
                   &power_active_phase1);
      }
    }

    if (!(channel_flags & SUPLA_CHANNEL_FLAG_PHASE2_UNSUPPORTED)) {
      if (em_ev_raw.measured_values & EM_VAR_VOLTAGE) {
        add_sample(lower_voltage_threshold, upper_voltage_threshold,
                   em_ev->get_voltage(2), &aberration_phase2);

        add_sample(voltage_logger_enabled, em_ev->get_voltage(2),
                   &voltage_phase2);
      }

      if ((em_ev_raw.measured_values & EM_VAR_CURRENT) ||
          (em_ev_raw.measured_values & EM_VAR_CURRENT_OVER_65A)) {
        add_sample(current_logger_enabled, em_ev->get_current(2),
                   &current_phase2);
      }

      if (em_ev_raw.measured_values & EM_VAR_POWER_ACTIVE) {
        add_sample(power_logger_enabled, em_ev->get_power_active(2),
                   &power_active_phase2);
      }
    }

    if (!(channel_flags & SUPLA_CHANNEL_FLAG_PHASE3_UNSUPPORTED)) {
      if (em_ev_raw.measured_values & EM_VAR_VOLTAGE) {
        add_sample(lower_voltage_threshold, upper_voltage_threshold,
                   em_ev->get_voltage(3), &aberration_phase3);
        add_sample(voltage_logger_enabled, em_ev->get_voltage(3),
                   &voltage_phase3);
      }

      if ((em_ev_raw.measured_values & EM_VAR_CURRENT) ||
          (em_ev_raw.measured_values & EM_VAR_CURRENT_OVER_65A)) {
        add_sample(current_logger_enabled, em_ev->get_current(3),
                   &current_phase3);
      }

      if (em_ev_raw.measured_values & EM_VAR_POWER_ACTIVE) {
        add_sample(power_logger_enabled, em_ev->get_power_active(3),
                   &power_active_phase3);
      }
    }
  }
}

void supla_electricity_analyzer::add_sample(supla_channel_value *value,
                                            supla_json_config *config) {}

bool supla_electricity_analyzer::is_any_aberration_for_logging_purpose(void) {
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

bool supla_electricity_analyzer::is_any_voltage_for_logging_purpose(void) {
  if (voltage_phase1 && voltage_phase1->get_sample_count() > 0) {
    return true;
  }

  if (voltage_phase2 && voltage_phase2->get_sample_count() > 0) {
    return true;
  }

  if (voltage_phase3 && voltage_phase3->get_sample_count() > 0) {
    return true;
  }

  return false;
}

bool supla_electricity_analyzer::is_any_current_for_logging_purpose(void) {
  if (current_phase1 && current_phase1->get_sample_count() > 0) {
    return true;
  }

  if (current_phase2 && current_phase2->get_sample_count() > 0) {
    return true;
  }

  if (current_phase3 && current_phase3->get_sample_count() > 0) {
    return true;
  }

  return false;
}

bool supla_electricity_analyzer::is_any_power_active_for_logging_purpose(void) {
  if (power_active_phase1 && power_active_phase1->get_sample_count() > 0) {
    return true;
  }

  if (power_active_phase2 && power_active_phase2->get_sample_count() > 0) {
    return true;
  }

  if (power_active_phase3 && power_active_phase3->get_sample_count() > 0) {
    return true;
  }

  return false;
}

bool supla_electricity_analyzer::is_any_data_for_logging_purpose(void) {
  return is_any_aberration_for_logging_purpose() ||
         is_any_voltage_for_logging_purpose() ||
         is_any_current_for_logging_purpose() ||
         is_any_power_active_for_logging_purpose();
}

void supla_electricity_analyzer::reset_aberrations(void) {
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

void supla_electricity_analyzer::reset_voltage(void) {
  if (voltage_phase1) {
    voltage_phase1->reset();
  }

  if (voltage_phase2) {
    voltage_phase2->reset();
  }

  if (voltage_phase3) {
    voltage_phase3->reset();
  }
}

void supla_electricity_analyzer::reset_current(void) {
  if (current_phase1) {
    current_phase1->reset();
  }

  if (current_phase2) {
    current_phase2->reset();
  }

  if (current_phase3) {
    current_phase3->reset();
  }
}

void supla_electricity_analyzer::reset_power_active(void) {
  if (power_active_phase1) {
    power_active_phase1->reset();
  }

  if (power_active_phase2) {
    power_active_phase2->reset();
  }

  if (power_active_phase3) {
    power_active_phase3->reset();
  }
}

void supla_electricity_analyzer::reset(void) {
  reset_aberrations();
  reset_voltage();
  reset_current();
  reset_power_active();
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

  if (voltage_phase1) {
    result->voltage_phase1 = new supla_simple_statiscics();
    *result->voltage_phase1 = *voltage_phase1;
  }

  if (voltage_phase2) {
    result->voltage_phase2 = new supla_simple_statiscics();
    *result->voltage_phase2 = *voltage_phase2;
  }

  if (voltage_phase3) {
    result->voltage_phase3 = new supla_simple_statiscics();
    *result->voltage_phase3 = *voltage_phase3;
  }

  if (current_phase1) {
    result->current_phase1 = new supla_simple_statiscics();
    *result->current_phase1 = *current_phase1;
  }

  if (current_phase2) {
    result->current_phase2 = new supla_simple_statiscics();
    *result->current_phase2 = *current_phase2;
  }

  if (current_phase3) {
    result->current_phase3 = new supla_simple_statiscics();
    *result->current_phase3 = *current_phase3;
  }

  if (power_active_phase1) {
    result->power_active_phase1 = new supla_simple_statiscics();
    *result->power_active_phase1 = *power_active_phase1;
  }

  if (power_active_phase2) {
    result->power_active_phase2 = new supla_simple_statiscics();
    *result->power_active_phase2 = *power_active_phase2;
  }

  if (power_active_phase3) {
    result->power_active_phase3 = new supla_simple_statiscics();
    *result->power_active_phase3 = *power_active_phase3;
  }

  return result;
}
