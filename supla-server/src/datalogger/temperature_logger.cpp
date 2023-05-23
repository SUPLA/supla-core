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

#include "datalogger/temperature_logger.h"

#include <stdio.h>

#include <memory>

#include "datalogger/temperature_logger_dao.h"
#include "device/device.h"
#include "safearray.h"

using std::shared_ptr;
using std::vector;
using std::weak_ptr;

supla_temperature_logger::supla_temperature_logger()
    : supla_abstract_cyclictask() {}

supla_temperature_logger::~supla_temperature_logger() {}

unsigned int supla_temperature_logger::task_interval_sec(void) { return 600; }

void supla_temperature_logger::run(const vector<supla_user *> *users,
                                   supla_abstract_db_access_provider *dba) {
  std::vector<supla_channel_value_envelope *> env;

  supla_temperature_logger_dao dao(dba);

  for (auto uit = users->cbegin(); uit != users->cend(); ++uit) {
    (*uit)->get_devices()->for_each(
        [&env](shared_ptr<supla_device> device, bool *will_continue) -> void {
          device->get_channels()->get_channel_values(
              &env, [](supla_channel_value *value) -> bool {
                return dynamic_cast<supla_channel_temphum_value *>(value) !=
                       nullptr;
              });
        });

    dao.load((*uit)->getUserID(), &env);
  }

  for (auto it = env.cbegin(); it != env.cend(); ++it) {
    supla_channel_temphum_value *th =
        dynamic_cast<supla_channel_temphum_value *>((*it)->get_value());

    if (th) {
      if (th->is_humidity_available()) {
        if (th->get_temperature() >
                supla_channel_temphum_value::incorrect_temperature() ||
            th->get_humidity() >
                supla_channel_temphum_value::incorrect_humidity()) {
          dao.add_temperature_and_humidity((*it)->get_channel_id(),
                                           th->get_temperature(),
                                           th->get_humidity());
        }
      } else if (th->get_temperature() >
                 supla_channel_temphum_value::incorrect_temperature()) {
        dao.add_temperature((*it)->get_channel_id(), th->get_temperature());
      }
    }

    delete *it;
  }
}
