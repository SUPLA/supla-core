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

#include "active_period.h"

#include <chrono>  // NOLINT

#include "astro/SolarCalculator.h"
#include "date/tz.h"
#include "json/cJSON.h"
#include "log.h"

using std::string;
using std::stringstream;

supla_active_period::supla_active_period(void) {
  this->active_from_utc = 0;
  this->active_to_utc = 0;
}

supla_active_period::~supla_active_period(void) {}

void supla_active_period::set_active_from(unsigned long utc) {
  active_from_utc = utc;
}

void supla_active_period::set_active_to(unsigned long utc) {
  active_to_utc = utc;
}

void supla_active_period::set_active_hours(string hours) {
  active_hours.clear();
  if (hours.front() != ',' || hours.back() != ',') {
    return;
  }

  hours = hours.substr(1, hours.length() - 2);
  stringstream ss(hours);

  std::string item;
  while (getline(ss, item, ',')) {
    unsigned char day_of_week = std::stoi(item.substr(0, 1));
    unsigned char hour = std::stoi(item.substr(1, item.length() - 1));
    if (day_of_week >= 1 && day_of_week <= 7 && hour >= 0 && hour <= 23) {
      active_hours[day_of_week].push_back(hour);
    }
  }
}

void supla_active_period::set_astro_conditions(const char *json) {
  astro_conditions.clear();
  if (!json) {
    return;
  }

  cJSON *root = cJSON_Parse(json);
  if (!root) {
    return;
  }

  if (cJSON_IsArray(root)) {
    for (int a = 0; a < cJSON_GetArraySize(root); a++) {
      cJSON *cnds = cJSON_GetArrayItem(root, a);
      if (cnds && cJSON_IsArray(cnds)) {
        std::vector<_t_astro_condition> and_cnds;

        for (int b = 0; b < cJSON_GetArraySize(cnds); b++) {
          cJSON *cnd = cJSON_GetArrayItem(cnds, b);
          if (cnd && cJSON_IsObject(cnd)) {
            cJSON *value = nullptr;
            _t_astro_condition astro_cnd = {};

            if ((value = cJSON_GetObjectItem(cnd, "beforeSunset"))) {
              astro_cnd.cnd_type = beforeSunset;
            } else if ((value = cJSON_GetObjectItem(cnd, "beforeSunrise"))) {
              astro_cnd.cnd_type = beforeSunrise;
            } else if ((value = cJSON_GetObjectItem(cnd, "afterSunset"))) {
              astro_cnd.cnd_type = afterSunset;
            } else if ((value = cJSON_GetObjectItem(cnd, "afterSunrise"))) {
              astro_cnd.cnd_type = afterSunrise;
            }

            if (value && cJSON_IsNumber(value) && value->valuedouble >= -1440 &&
                value->valuedouble <= 1440) {
              astro_cnd.minutes = value->valuedouble;
              and_cnds.push_back(astro_cnd);
            }
          }
        }

        if (and_cnds.size()) {
          astro_conditions.push_back(and_cnds);
        }
      }
    }
  }

  cJSON_Delete(root);
}

std::chrono::time_point<std::chrono::system_clock>
supla_active_period::get_current_point_in_time(void) {
  return std::chrono::system_clock::now();
}

bool supla_active_period::_is_now_active(const char *timezone, double latitude,
                                         double longitude) {
  auto now =
      date::make_zoned(timezone ? timezone : "", get_current_point_in_time());

  unsigned long now_utc = std::chrono::duration_cast<std::chrono::seconds>(
                              now.get_sys_time().time_since_epoch())
                              .count();

  if (active_from_utc && active_from_utc > now_utc) {
    return false;
  }

  if (active_to_utc && active_to_utc < now_utc) {
    return false;
  }

  auto midnight_time_point = date::floor<date::days>(now.get_local_time());
  auto now_time_point = date::floor<std::chrono::seconds>(now.get_local_time());

  if (active_hours.size()) {
    auto time_diff = date::make_time(midnight_time_point - now_time_point);
    unsigned char hour = time_diff.hours().count();
    int week_day = date::weekday{midnight_time_point}.iso_encoding();

    bool hour_found = false;

    auto ah_it = active_hours.find(week_day);
    if (ah_it != active_hours.end()) {
      if (std::find(ah_it->second.begin(), ah_it->second.end(), hour) !=
          ah_it->second.end()) {
        hour_found = true;
      }
    }

    if (!hour_found) {
      return false;
    }
  }

  if (astro_conditions.size()) {
    bool any_contitions_met = false;

    double transit = 0.0;
    double sunrise = 0.0;
    double sunset = 0.0;

    auto midnight_time_point_utc = date::floor<date::days>(now.get_sys_time());

    unsigned long midnight_utc =
        std::chrono::duration_cast<std::chrono::seconds>(
            midnight_time_point_utc.time_since_epoch())
            .count();

    calcSunriseSunset(now_utc, latitude, longitude, transit, sunrise, sunset,
                      SUNRISESET_STD_ALTITUDE, 1);

    sunset *= 3600;
    sunrise *= 3600;

    for (auto it1 = astro_conditions.begin(); it1 != astro_conditions.end();
         ++it1) {
      bool all_contitions_met = true;

      for (auto it2 = it1->begin(); it2 != it1->end(); ++it2) {
        unsigned long timestamp = midnight_utc;
        timestamp +=
            it2->cnd_type == afterSunrise || it2->cnd_type == beforeSunrise
                ? sunrise
                : sunset;
        timestamp += it2->minutes * 60;

        if (((it2->cnd_type == afterSunrise || it2->cnd_type == afterSunset) &&
             timestamp >= now_utc) ||
            ((it2->cnd_type == beforeSunrise ||
              it2->cnd_type == beforeSunset) &&
             timestamp <= now_utc)) {
          all_contitions_met = false;
          break;
        }
      }

      if (it1->size() && all_contitions_met) {
        any_contitions_met = true;
      }
    }

    if (!any_contitions_met) {
      return false;
    }
  }

  return true;
}

bool supla_active_period::is_now_active(const char *timezone, double latitude,
                                        double longitude) {
  bool result = false;
  try {
    result = _is_now_active(timezone, latitude, longitude);
  } catch (std::exception &e) {
    supla_log(
        LOG_ERR,
        "The is_now_active method throws an exception with the message: %s",
        e.what());
  }
  return result;
}
