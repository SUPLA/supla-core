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

using std::shared_ptr;
using std::vector;

supla_electricity_logger::supla_electricity_logger()
    : supla_abstract_cyclictask() {}

supla_electricity_logger::~supla_electricity_logger() {}

unsigned int supla_electricity_logger::task_interval_sec(void) { return 600; }

void supla_electricity_logger::run(const vector<supla_user *> *users,
                                   supla_abstract_db_access_provider *dba) {
  vector<supla_channel_electricity_measurement *> em;

  for (auto uit = users->cbegin(); uit != users->cend(); ++uit) {
    vector<shared_ptr<supla_device> > devices =
        (*uit)->get_devices()->get_all();

    for (auto dit = devices.cbegin(); dit != devices.cend(); ++dit) {
      (*dit)->get_channels()->get_electricity_measurements(&em, true);
    }
  }

  supla_electricity_logger_dao dao(dba);

  for (auto it = em.cbegin(); it != em.cend(); ++it) {
    dao.add(*it);
    delete *it;
  }
}
