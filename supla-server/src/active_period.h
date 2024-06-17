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

#ifndef ACTIVE_PERIOD_H_
#define ACTIVE_PERIOD_H_

#include <chrono>  // NOLINT
#include <map>
#include <string>
#include <vector>

class supla_active_period {
 private:
  enum _cnd_type_e { afterSunset, beforeSunset, afterSunrise, beforeSunrise };

  typedef struct {
    _cnd_type_e cnd_type;
    int minutes;
  } _t_astro_condition;

  unsigned long active_from_utc;
  unsigned long active_to_utc;
  std::map<unsigned char, std::vector<unsigned char>> active_hours;
  std::vector<std::vector<_t_astro_condition>> astro_conditions;

  bool _is_now_active(const char *timezone, double latitude,
                      double longitude) const;
  virtual std::chrono::time_point<std::chrono::system_clock>
  get_current_point_in_time(void) const;

 public:
  supla_active_period(void);
  supla_active_period(unsigned long active_from, unsigned long active_to,
                      const char *active_hours, const char *astro_conditions);
  virtual ~supla_active_period(void);

  void set_active_from(unsigned long utc);
  void set_active_to(unsigned long utc);
  void set_active_hours(std::string hours);
  void set_astro_conditions(const char *json);

  bool is_now_active(const char *timezone, double latitude,
                     double longitude) const;

  bool operator==(const supla_active_period &ap) const;
};

#endif /* ACTIVE_PERIOD_H_ */
