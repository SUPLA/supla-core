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

#ifndef ABSTRACT_AUTODISCOVER_STATISTICS_H_
#define ABSTRACT_AUTODISCOVER_STATISTICS_H_

#include <string>
#include <vector>

#include "cyclictasks/abstract_cyclictask.h"

#define AUTODISCOVER_STATISTICS_ENDPOINT_URL \
  "https://autodiscover.supla.org/server-stats"

class supla_abstract_curl_adapter;

class supla_abstract_autodiscover_statistics
    : public supla_abstract_cyclictask {
 private:
  bool first_run;
  bool initial_delay_initialized;
  unsigned int initial_delay_interval_sec;
  unsigned int initial_delay_sec;

 public:
  typedef struct {
    int channel_count;
    double power_active;
    double power_reactive;
    double power_apparent;
  } electricity_meter_statistics_t;

 protected:
  virtual unsigned int task_interval_sec(void);
  virtual void run(const std::vector<supla_user *> *users,
                   supla_abstract_db_access_provider *dba);
  virtual bool user_access_needed(void);

  virtual unsigned int get_cfg_interval_sec(void);
  virtual unsigned int get_initial_delay_sec(unsigned int interval_sec);
  virtual const char *get_statistics_endpoint_url(void);
  virtual std::string get_generated_at(void);
  virtual std::string get_target_token(
      supla_abstract_db_access_provider *dba) = 0;
  virtual supla_abstract_curl_adapter *get_curl_adapter(void) = 0;
  virtual void release_curl_adapter(supla_abstract_curl_adapter *adapter) = 0;

  electricity_meter_statistics_t collect_statistics(
      const std::vector<supla_user *> *users);
  std::string get_payload(const electricity_meter_statistics_t *statistics);
  bool post_statistics(supla_abstract_curl_adapter *curl_adapter,
                       const std::string &target_token,
                       const electricity_meter_statistics_t *statistics);

 public:
  supla_abstract_autodiscover_statistics(void);
  virtual ~supla_abstract_autodiscover_statistics();
};

#endif /* ABSTRACT_AUTODISCOVER_STATISTICS_H_ */
