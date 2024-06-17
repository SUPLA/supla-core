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

#ifndef WEEKLY_SCHEDULE_CONFIG_H_
#define WEEKLY_SCHEDULE_CONFIG_H_

#include <string>

#include "jsonconfig/json_config.h"
#include "proto.h"

class weekly_schedule_config : public supla_json_config {
 private:
  static const char mode[];
  static const char setpoint_temperature_heat[];
  static const char setpoint_temperature_cool[];
  static const char program_settings[];
  static const char quarters[];

  cJSON *get_ws_root(bool force);
  std::string mode_to_string(unsigned char mode);
  unsigned char string_to_mode(const std::string &mode);
  void add_program(unsigned char index, TChannelConfig_WeeklySchedule *config,
                   cJSON *program_root);
  bool get_program(unsigned char index, TChannelConfig_WeeklySchedule *config,
                   cJSON *program_root);
  void add_quarter(TChannelConfig_WeeklySchedule *config, cJSON *quarters_root,
                   unsigned char quarter);

 protected:
  virtual std::string get_weekly_shedule_key(void);

 public:
  explicit weekly_schedule_config(supla_json_config *root);
  weekly_schedule_config(void);
  virtual void merge(supla_json_config *dst);
  void set_config(TChannelConfig_WeeklySchedule *config);
  bool get_config(TChannelConfig_WeeklySchedule *config);
};

#endif /* WEEKLY_SCHEDULE_CONFIG_H_ */
