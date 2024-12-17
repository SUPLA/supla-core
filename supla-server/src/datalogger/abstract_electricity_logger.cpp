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

#include "abstract_electricity_logger.h"

#include <memory>

#include "device/devicechannel.h"
#include "user/user.h"

using std::shared_ptr;
using std::vector;
using std::weak_ptr;

supla_abstract_electricity_logger::supla_abstract_electricity_logger()
    : supla_abstract_cyclictask() {}

supla_abstract_electricity_logger::~supla_abstract_electricity_logger() {}

void supla_abstract_electricity_logger::run(
    const vector<supla_user *> *users, supla_abstract_db_access_provider *dba) {
  std::vector<supla_electricity_analyzer *> vel;

  for (auto uit = users->cbegin(); uit != users->cend(); ++uit) {
    (*uit)->get_devices()->for_each(
        [&vel, this](shared_ptr<supla_device> device,
                     bool *will_continue) -> void {
          device->get_channels()->for_each([&vel, this](
                                               supla_device_channel *channel,
                                               bool *will_continue) -> void {
            if (channel->get_availability_status().is_online()) {
              channel->access_data_analyzer(
                  [&vel, this](supla_abstract_data_analyzer *analyzer) -> void {
                    supla_electricity_analyzer *el_analyzer =
                        dynamic_cast<supla_electricity_analyzer *>(analyzer);
                    if (el_analyzer &&
                        is_any_data_for_logging_purposes(el_analyzer)) {
                      supla_abstract_data_analyzer *copy = analyzer->copy();
                      if (copy) {
                        supla_electricity_analyzer *el_analyzer_copy =
                            dynamic_cast<supla_electricity_analyzer *>(copy);
                        if (el_analyzer_copy) {
                          vel.push_back(el_analyzer_copy);
                          reset(el_analyzer);
                        } else {
                          delete copy;
                        }
                      }
                    }
                  });
            }
          });
        });
  }

  if (vel.size()) {
    supla_abstract_electricity_logger_dao *dao = get_dao(dba);
    if (dao) {
      for (auto it = vel.begin(); it != vel.end(); ++it) {
        dao->add(*it);
        delete *it;
      }
      delete dao;
    }
  }
}
