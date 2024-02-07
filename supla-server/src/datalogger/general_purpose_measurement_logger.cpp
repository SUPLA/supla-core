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

#include "datalogger/general_purpose_measurement_logger.h"

#include <memory>

#include "datalogger/general_purpose_measurement_logger_dao.h"
#include "device/devicechannel.h"
#include "user/user.h"

using std::shared_ptr;
using std::vector;
using std::weak_ptr;

supla_general_purpose_measurement_logger::
    supla_general_purpose_measurement_logger()
    : supla_abstract_cyclictask() {}

supla_general_purpose_measurement_logger::
    ~supla_general_purpose_measurement_logger() {}

unsigned int supla_general_purpose_measurement_logger::task_interval_sec(void) {
  return 600;
}

void supla_general_purpose_measurement_logger::run(
    const vector<supla_user *> *users, supla_abstract_db_access_provider *dba) {
  std::vector<supla_general_purpose_measurement_analyzer *> analyzers;

  for (auto uit = users->cbegin(); uit != users->cend(); ++uit) {
    (*uit)->get_devices()->for_each([&analyzers, this](
                                        shared_ptr<supla_device> device,
                                        bool *will_continue) -> void {
      device->get_channels()->for_each([&analyzers, this](
                                           supla_device_channel *channel,
                                           bool *will_continue) -> void {
        channel->access_data_analyzer([&analyzers,
                                       this](supla_abstract_data_analyzer
                                                 *analyzer) -> void {
          supla_general_purpose_measurement_analyzer *gpm_analyzer =
              dynamic_cast<supla_general_purpose_measurement_analyzer *>(
                  analyzer);
          if (gpm_analyzer) {
            supla_abstract_data_analyzer *copy = analyzer->copy();
            if (copy) {
              supla_general_purpose_measurement_analyzer *gpm_analyzer_copy =
                  dynamic_cast<supla_general_purpose_measurement_analyzer *>(
                      copy);
              if (gpm_analyzer_copy) {
                analyzers.push_back(gpm_analyzer_copy);
                analyzer->reset();
              } else {
                delete copy;
              }
            }
          }
        });
      });
    });
  }

  supla_general_purpose_measurement_logger_dao dao(dba);

  for (auto it = analyzers.begin(); it != analyzers.end(); ++it) {
    dao.add(*it);
    delete *it;
  }
}
