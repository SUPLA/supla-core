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

#include "datalogger/thermostat_logger.h"

#include <memory>

#include "datalogger/thermostat_logger_dao.h"
#include "device/device.h"
#include "safearray.h"

using std::shared_ptr;
using std::vector;
using std::weak_ptr;

supla_thermostat_logger::supla_thermostat_logger()
    : supla_abstract_cyclictask() {}

supla_thermostat_logger::~supla_thermostat_logger() {}

unsigned int supla_thermostat_logger::task_interval_sec(void) { return 600; }

void supla_thermostat_logger::run(const vector<supla_user *> *users,
                                  supla_abstract_db_access_provider *dba) {
  std::vector<supla_channel_thermostat_measurement *> th;
  for (auto uit = users->cbegin(); uit != users->cend(); ++uit) {
    (*uit)->get_devices()->for_each(
        [&th](shared_ptr<supla_device> device, bool *will_continue) -> void {
          device->get_channels()->get_thermostat_measurements(&th);
        });
  }

  supla_thermostat_logger_dao dao(dba);

  for (auto it = th.cbegin(); it != th.cend(); ++it) {
    if ((*it)->getMeasuredTemperature() > -273) {
      dao.add(*it);
    }

    delete *it;
  }
}
