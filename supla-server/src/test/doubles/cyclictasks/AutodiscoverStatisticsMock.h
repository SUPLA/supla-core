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

#ifndef AUTODISCOVER_STATISTICS_MOCK_H_
#define AUTODISCOVER_STATISTICS_MOCK_H_

#include <gmock/gmock.h>

#include <string>
#include <vector>

#include "cyclictasks/abstract_autodiscover_statistics.h"

namespace testing {

class AutodiscoverStatisticsMock
    : public supla_abstract_autodiscover_statistics {
 public:
  AutodiscoverStatisticsMock(void);
  virtual ~AutodiscoverStatisticsMock(void);

  MOCK_METHOD1(get_target_token,
               std::string(supla_abstract_db_access_provider *dba));
  MOCK_METHOD0(get_curl_adapter, supla_abstract_curl_adapter *(void));
  MOCK_METHOD1(release_curl_adapter,
               void(supla_abstract_curl_adapter *adapter));
  MOCK_METHOD0(get_cfg_interval_sec, unsigned int(void));
  MOCK_METHOD1(get_initial_delay_sec, unsigned int(unsigned int interval_sec));
  MOCK_METHOD0(get_generated_at, std::string(void));

  void run(const std::vector<supla_user *> *users,
           supla_abstract_db_access_provider *dba);
  unsigned int task_interval_sec(void);
  electricity_meter_statistics_t collect_statistics(
      const std::vector<supla_user *> *users);
  std::string get_payload(const electricity_meter_statistics_t *statistics);
  bool post_statistics(supla_abstract_curl_adapter *curl_adapter,
                       const std::string &target_token,
                       const electricity_meter_statistics_t *statistics);
};

} /* namespace testing */

#endif /* AUTODISCOVER_STATISTICS_MOCK_H_ */
