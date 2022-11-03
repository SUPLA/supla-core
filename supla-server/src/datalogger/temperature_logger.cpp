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

supla_temperature_logger::supla_temperature_logger()
    : supla_abstract_cyclictask() {}

supla_temperature_logger::~supla_temperature_logger() {}

unsigned int supla_temperature_logger::task_interval_sec(void) { return 600; }

void supla_temperature_logger::run(const vector<supla_user *> *users,
                                   supla_abstract_db_access_provider *dba) {
  std::vector<supla_channel_temphum *> th;

  supla_temperature_logger_dao dao(dba);

  for (auto uit = users->cbegin(); uit != users->cend(); ++uit) {
    vector<shared_ptr<supla_device> > devices =
        (*uit)->get_devices()->get_all();

    for (auto dit = devices.cbegin(); dit != devices.cend(); ++dit) {
      (*dit)->get_channels()->get_temp_and_humidity(&th);
    }

    dao.load((*uit)->getUserID(), &th);
  }

  for (auto it = th.cbegin(); it != th.cend(); ++it) {
    if ((*it)->isTempAndHumidity() == 1) {
      if ((*it)->getTemperature() > -273 || (*it)->getHumidity() > -1) {
        dao.add_temperature_and_humidity((*it)->getChannelId(),
                                         (*it)->getTemperature(),
                                         (*it)->getHumidity());
      }
    } else if ((*it)->getTemperature() > -273) {
      dao.add_temperature((*it)->getChannelId(), (*it)->getTemperature());
    }

    delete *it;
  }
}
