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

#include "datalogger/general_purpose_text_logger.h"

#include <memory>
#include <vector>

#include "datalogger/general_purpose_text_logger_dao.h"
#include "device/device.h"
#include "device/extended_value/channel_general_purpose_text_extended_value.h"
#include "user/user.h"

using std::shared_ptr;
using std::vector;

supla_general_purpose_text_logger::supla_general_purpose_text_logger()
    : supla_abstract_cyclictask() {}

supla_general_purpose_text_logger::~supla_general_purpose_text_logger() {}

bool supla_general_purpose_text_logger::is_tsdb_preffered(void) {
  return false;
}

unsigned int supla_general_purpose_text_logger::task_interval_sec(void) {
  return 60;
}

void supla_general_purpose_text_logger::run(
    const vector<supla_user *> *users, supla_abstract_db_access_provider *dba) {
  std::vector<supla_abstract_channel_extended_value_envelope *> env;

  supla_general_purpose_text_logger_dao dao(dba);

  for (auto uit = users->cbegin(); uit != users->cend(); ++uit) {
    (*uit)->get_devices()->for_each(
        [&env](shared_ptr<supla_device> device, bool *will_continue) -> void {
          device->get_channels()->get_channel_extended_values(
              &env,
              [](supla_abstract_channel_extended_value *value) -> bool {
                return supla_channel_general_purpose_text_extended_value::
                    is_ev_type_supported(value->get_type());
              },
              true);
        });
  }

  for (auto it = env.cbegin(); it != env.cend(); ++it) {
    supla_channel_general_purpose_text_extended_value *gpt_ev =
        dynamic_cast<supla_channel_general_purpose_text_extended_value *>(
            (*it)->get_extended_value());

    if (gpt_ev) {
      dao.add((*it)->get_channel_id(), gpt_ev->get_text());
    }

    delete *it;
  }
}
