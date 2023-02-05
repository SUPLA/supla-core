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

#include "datalogger/electricity_logger.h"

#include <memory>

#include "datalogger/electricity_logger_dao.h"
#include "device/device.h"
#include "safearray.h"

using std::function;
using std::shared_ptr;
using std::vector;
using std::weak_ptr;

supla_electricity_logger::supla_electricity_logger()
    : supla_abstract_cyclictask() {}

supla_electricity_logger::~supla_electricity_logger() {}

unsigned int supla_electricity_logger::task_interval_sec(void) { return 600; }

void supla_electricity_logger::run(const vector<supla_user *> *users,
                                   supla_abstract_db_access_provider *dba) {
  vector<supla_channel_electricity_measurement *> em;

  for (auto uit = users->cbegin(); uit != users->cend(); ++uit) {
    (*uit)->get_devices()->for_each(
        [&em](shared_ptr<supla_device> device, bool *will_continue) -> void {
          device->get_channels()->get_electricity_measurements(&em, true);
        });
  }

  supla_electricity_logger_dao dao(dba);

  for (auto it = em.cbegin(); it != em.cend(); ++it) {
    TElectricityMeter_ExtendedValue_V2 em_ev = {};
    (*it)->getMeasurement(&em_ev);

    bool ne_zero = false;

    for (int a = 0; a < 3; a++) {
      if (em_ev.total_forward_active_energy[a] ||
          em_ev.total_reverse_active_energy[a] ||
          em_ev.total_forward_reactive_energy[a] ||
          em_ev.total_reverse_reactive_energy[a]) {
        ne_zero = true;
        break;
      }
    }

    if (ne_zero || em_ev.total_forward_active_energy_balanced ||
        em_ev.total_reverse_active_energy_balanced) {
      dao.add((*it)->getChannelId(), &em_ev);
    }

    delete *it;
  }
}
