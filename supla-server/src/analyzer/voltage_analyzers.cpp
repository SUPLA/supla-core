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

#include "analyzer/voltage_analyzers.h"

#include "srpc/srpc.h"

supla_voltage_analyzers::supla_voltage_analyzers() {
  channel_id = 0;
  phase1 = nullptr;
  phase2 = nullptr;
  phase3 = nullptr;
}

supla_voltage_analyzers::supla_voltage_analyzers(
    const supla_voltage_analyzers &analyzers) {
  channel_id = 0;
  phase1 = nullptr;
  phase2 = nullptr;
  phase3 = nullptr;
  *this = analyzers;
}

supla_voltage_analyzers::~supla_voltage_analyzers(void) {
  if (phase1) {
    delete phase1;
  }

  if (phase2) {
    delete phase2;
  }

  if (phase3) {
    delete phase3;
  }
}

void supla_voltage_analyzers::set_channel_id(int channel_id) {
  this->channel_id = channel_id;
}

int supla_voltage_analyzers::get_channel_id(void) { return channel_id; }

supla_voltage_analyzer *supla_voltage_analyzers::get_phase1(void) {
  return phase1;
}

supla_voltage_analyzer *supla_voltage_analyzers::get_phase2(void) {
  return phase2;
}

supla_voltage_analyzer *supla_voltage_analyzers::get_phase3(void) {
  return phase3;
}

void supla_voltage_analyzers::add_sample(double lower_voltage_threshold,
                                         double upper_voltage_threshold,
                                         double voltage,
                                         supla_voltage_analyzer **va) {
  if (*va == nullptr) {
    *va = new supla_voltage_analyzer();
    (*va)->set_lower_threshold(lower_voltage_threshold);
    (*va)->set_upper_threshold(upper_voltage_threshold);
  }

  (*va)->add_sample(voltage);
}

void supla_voltage_analyzers::add_samples(
    int channel_flags, electricity_meter_config *config,
    TSuplaChannelExtendedValue *extendedValue) {
  double lower_voltage_threshold = 0;
  double upper_voltage_threshold = 0;

  if (!config || !extendedValue) {
    return;
  }

  lower_voltage_threshold = config->get_lower_voltage_threshold();
  upper_voltage_threshold = config->get_upper_voltage_threshold();

  if (!lower_voltage_threshold && !upper_voltage_threshold) {
    return;
  }

  TElectricityMeter_ExtendedValue_V2 em_ev = {};
  if (srpc_evtool_v2_extended2emextended(extendedValue, &em_ev) &&
      (em_ev.measured_values & EM_VAR_VOLTAGE) && em_ev.m_count > 0) {
    if (!(channel_flags & SUPLA_CHANNEL_FLAG_PHASE1_UNSUPPORTED)) {
      add_sample(lower_voltage_threshold, upper_voltage_threshold,
                 em_ev.m[0].voltage[0] / 100.00, &phase1);
    }

    if (!(channel_flags & SUPLA_CHANNEL_FLAG_PHASE2_UNSUPPORTED)) {
      add_sample(lower_voltage_threshold, upper_voltage_threshold,
                 em_ev.m[0].voltage[1] / 100.00, &phase2);
    }

    if (!(channel_flags & SUPLA_CHANNEL_FLAG_PHASE3_UNSUPPORTED)) {
      add_sample(lower_voltage_threshold, upper_voltage_threshold,
                 em_ev.m[0].voltage[2] / 100.00, &phase3);
    }
  }
}

bool supla_voltage_analyzers::is_any_sample_over_threshold(void) {
  if (phase1 &&
      (phase1->get_lower_counter() > 0 || phase1->get_upper_counter() > 0)) {
    return true;
  }

  if (phase2 &&
      (phase2->get_lower_counter() > 0 || phase2->get_upper_counter() > 0)) {
    return true;
  }
  if (phase3 &&
      (phase3->get_lower_counter() > 0 || phase3->get_upper_counter() > 0)) {
    return true;
  }

  return false;
}

void supla_voltage_analyzers::reset(void) {
  if (phase1) {
    phase1->reset();
  }

  if (phase2) {
    phase2->reset();
  }

  if (phase3) {
    phase3->reset();
  }
}

supla_voltage_analyzers &supla_voltage_analyzers::operator=(
    const supla_voltage_analyzers &analyzers) {
  channel_id = analyzers.channel_id;

  if (phase1 && !analyzers.phase1) {
    delete phase1;
    phase1 = nullptr;
  }

  if (phase2 && !analyzers.phase2) {
    delete phase2;
    phase2 = nullptr;
  }

  if (phase3 && !analyzers.phase3) {
    delete phase3;
    phase3 = nullptr;
  }

  if (analyzers.phase1) {
    if (!phase1) {
      phase1 = new supla_voltage_analyzer();
    }
    *phase1 = *analyzers.phase1;
  }

  if (analyzers.phase2) {
    if (!phase2) {
      phase2 = new supla_voltage_analyzer();
    }
    *phase2 = *analyzers.phase2;
  }

  if (analyzers.phase3) {
    if (!phase3) {
      phase3 = new supla_voltage_analyzer();
    }
    *phase3 = *analyzers.phase3;
  }

  return *this;
}
