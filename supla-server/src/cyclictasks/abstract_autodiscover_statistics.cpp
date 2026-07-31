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

#include "cyclictasks/abstract_autodiscover_statistics.h"

#include <time.h>

#include <cmath>
#include <memory>
#include <random>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "device/device.h"
#include "device/devicechannels.h"
#include "device/extended_value/channel_em_extended_value.h"
#include "http/abstract_curl_adapter.h"
#include "log.h"
#include "proto.h"
#include "svrcfg.h"
#include "user/user.h"

using std::shared_ptr;
using std::string;
using std::vector;

namespace {
const unsigned int MIN_INTERVAL_SEC = 60;

void add_directional_power(double value, double *forward, double *reverse) {
  if (value < 0) {
    *reverse += std::abs(value);
  } else if (value > 0) {
    *forward += value;
  }
}
}

supla_abstract_autodiscover_statistics::
    supla_abstract_autodiscover_statistics()
    : supla_abstract_cyclictask(),
      first_run(true),
      initial_delay_initialized(false),
      initial_delay_interval_sec(0),
      initial_delay_sec(0) {}

supla_abstract_autodiscover_statistics::
    ~supla_abstract_autodiscover_statistics() {}

bool supla_abstract_autodiscover_statistics::user_access_needed(void) {
  return true;
}

unsigned int supla_abstract_autodiscover_statistics::get_cfg_interval_sec(
    void) {
  int interval_sec = scfg_int(CFG_AUTODISCOVER_STATISTICS_EXPORT_INTERVAL_SEC);

  if (interval_sec <= 0) {
    return 0;
  }

  if (interval_sec < static_cast<int>(MIN_INTERVAL_SEC)) {
    return MIN_INTERVAL_SEC;
  }

  return static_cast<unsigned int>(interval_sec);
}

unsigned int supla_abstract_autodiscover_statistics::get_initial_delay_sec(
    unsigned int interval_sec) {
  if (interval_sec < 2) {
    return 0;
  }

  std::random_device rd;
  std::mt19937 generator(rd());
  std::uniform_int_distribution<unsigned int> distribution(0, interval_sec - 1);

  return distribution(generator);
}

const char *
supla_abstract_autodiscover_statistics::get_statistics_endpoint_url(void) {
  return AUTODISCOVER_STATISTICS_ENDPOINT_URL;
}

string supla_abstract_autodiscover_statistics::get_generated_at(void) {
  time_t now = time(NULL);
  struct tm utc_time = {};
  char buffer[sizeof("2020-01-01T00:00:00.000Z")] = {};

  gmtime_r(&now, &utc_time);

  if (strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S.000Z",
               &utc_time) == 0) {
    return "";
  }

  return buffer;
}

unsigned int supla_abstract_autodiscover_statistics::task_interval_sec(void) {
  unsigned int interval_sec = get_cfg_interval_sec();

  if (interval_sec == 0) {
    interval_sec = MIN_INTERVAL_SEC;
  }

  if (first_run) {
    if (!initial_delay_initialized ||
        initial_delay_interval_sec != interval_sec) {
      initial_delay_sec = get_initial_delay_sec(interval_sec);
      initial_delay_interval_sec = interval_sec;
      initial_delay_initialized = true;
    }

    return initial_delay_sec;
  }

  return interval_sec;
}

supla_abstract_autodiscover_statistics::electricity_meter_statistics_t
supla_abstract_autodiscover_statistics::collect_statistics(
    const vector<supla_user *> *users) {
  electricity_meter_statistics_t result = {};

  if (!users) {
    return result;
  }

  for (auto uit = users->cbegin(); uit != users->cend(); ++uit) {
    (*uit)->get_devices()->for_each(
        [&result](shared_ptr<supla_device> device,
                  bool *will_continue) -> void {
          (void)will_continue;

          if (device->get_manufacturer_id() == 0) {
            return;
          }

          device->get_channels()->for_each(
              [&result](supla_device_channel *channel,
                        bool *will_continue) -> void {
                (void)will_continue;

                if (channel->get_func() != SUPLA_CHANNELFNC_ELECTRICITY_METER ||
                    !channel->get_availability_status().is_online()) {
                  return;
                }

                supla_channel_em_extended_value *em_extended_value =
                    channel->get_extended_value<
                        supla_channel_em_extended_value>(false);

                result.channel_count++;

                if (em_extended_value) {
                  add_directional_power(
                      em_extended_value->get_power_active_kw_sum(),
                      &result.power_active_forward_kw,
                      &result.power_active_reverse_kw);

                  delete em_extended_value;
                }
              });
        });
  }

  return result;
}

string supla_abstract_autodiscover_statistics::get_payload(
    const electricity_meter_statistics_t *statistics) {
  electricity_meter_statistics_t empty = {};

  if (!statistics) {
    statistics = &empty;
  }

  nlohmann::json payload = {
      {"generatedAt", get_generated_at()},
      {"statistics",
       {{"electricityMeters",
         {{"powerActiveForwardKW", statistics->power_active_forward_kw},
          {"powerActiveReverseKW", statistics->power_active_reverse_kw},
          {"channelCount", statistics->channel_count}}}}}};

  return payload.dump();
}

bool supla_abstract_autodiscover_statistics::post_statistics(
    supla_abstract_curl_adapter *curl_adapter, const string &target_token,
    const electricity_meter_statistics_t *statistics) {
  if (!curl_adapter || target_token.empty()) {
    return false;
  }

  string auth = "Authorization: Bearer ";
  auth.append(target_token);

  string payload = get_payload(statistics);
  string request_result;

  curl_adapter->reset();
  curl_adapter->set_opt_url(get_statistics_endpoint_url());
  curl_adapter->append_header("Content-Type: application/json");
  curl_adapter->append_header(auth.c_str());
  curl_adapter->set_opt_post_fields(payload.c_str());
  curl_adapter->set_opt_write_data(&request_result);

  if (!curl_adapter->perform()) {
    supla_log(LOG_ERR,
              "An attempt to export statistics to the AD server failed.");
    return false;
  }

  long response_code = curl_adapter->get_response_code();
  bool result =
      response_code == 200 || response_code == 202 || response_code == 204;

  if (!result) {
    supla_log(LOG_ERR,
              "An attempt to export statistics to the AD server failed. "
              "Result code: %i",
              response_code);
  }

  return result;
}

void supla_abstract_autodiscover_statistics::run(
    const vector<supla_user *> *users, supla_abstract_db_access_provider *dba) {
  first_run = false;

  if (get_cfg_interval_sec() == 0) {
    return;
  }

  string target_token = get_target_token(dba);
  if (target_token.empty()) {
    return;
  }

  electricity_meter_statistics_t statistics = collect_statistics(users);

  supla_abstract_curl_adapter *curl_adapter = get_curl_adapter();
  if (!curl_adapter) {
    return;
  }

  post_statistics(curl_adapter, target_token, &statistics);
  release_curl_adapter(curl_adapter);
}
