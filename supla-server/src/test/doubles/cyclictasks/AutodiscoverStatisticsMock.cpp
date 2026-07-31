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

#include "doubles/cyclictasks/AutodiscoverStatisticsMock.h"

#include <string>
#include <vector>

namespace testing {

using std::string;
using std::vector;

AutodiscoverStatisticsMock::AutodiscoverStatisticsMock(void)
    : supla_abstract_autodiscover_statistics() {}

AutodiscoverStatisticsMock::~AutodiscoverStatisticsMock(void) {}

void AutodiscoverStatisticsMock::run(const vector<supla_user *> *users,
                                     supla_abstract_db_access_provider *dba) {
  supla_abstract_autodiscover_statistics::run(users, dba);
}

unsigned int AutodiscoverStatisticsMock::task_interval_sec(void) {
  return supla_abstract_autodiscover_statistics::task_interval_sec();
}

AutodiscoverStatisticsMock::electricity_meter_statistics_t
AutodiscoverStatisticsMock::collect_statistics(
    const vector<supla_user *> *users) {
  return supla_abstract_autodiscover_statistics::collect_statistics(users);
}

string AutodiscoverStatisticsMock::get_payload(
    const electricity_meter_statistics_t *statistics) {
  return supla_abstract_autodiscover_statistics::get_payload(statistics);
}

bool AutodiscoverStatisticsMock::post_statistics(
    supla_abstract_curl_adapter *curl_adapter, const string &target_token,
    const electricity_meter_statistics_t *statistics) {
  return supla_abstract_autodiscover_statistics::post_statistics(
      curl_adapter, target_token, statistics);
}

}  // namespace testing
