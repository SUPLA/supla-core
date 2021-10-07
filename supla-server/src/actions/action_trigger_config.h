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

#ifndef ACTIONTRIGGERCONFIG_H_
#define ACTIONTRIGGERCONFIG_H_

#include <functional>
#include <string>

#include "channeljsonconfig/channeljsonconfig.h"

#define ACTION_OPEN 10
#define ACTION_CLOSE 20
#define ACTION_SHUT 30
#define ACTION_REVEAL 40
#define ACTION_REVEAL_PARTIALLY 50
#define ACTION_TURN_ON 60
#define ACTION_TURN_OFF 70
#define ACTION_SET_RGBW_PARAMETERS 80
#define ACTION_OPEN_CLOSE 90
#define ACTION_STOP 100
#define ACTION_TOGGLE 110

typedef struct {
  int cap;
  std::string str;
} _atc_map_t;

typedef struct {
  int actionId;
  bool channelGroup;
  int subjectId;
} _at_config_action_t;

typedef struct {
  char brightness;
  char color_brightness;
  unsigned int color;
  bool color_random;
} _at_config_rgbw_t;

class action_trigger_config : public channel_json_config {
 private:
  static const _atc_map_t map[];
  static const char caps_key[];
  static const char disables_local_operation_key[];
  static const char action_key[];
  static const char actions_key[];
  bool equal(const char *str1, const char *str2);
  bool equal(cJSON *item, const char *str);
  int to_cap(cJSON *item);
  const char *to_string(int cap);
  int to_cap(const char *str);
  cJSON *get_cap_user_config(int cap);
  unsigned int get_capabilities(const char *key);
  bool set_capabilities(const char *key, std::function<unsigned int()> get_caps,
                        unsigned int caps);

 public:
  explicit action_trigger_config(channel_json_config *root);
  action_trigger_config(void);

  unsigned int get_capabilities(void);
  bool set_capabilities(unsigned int caps);
  unsigned int get_caps_that_disables_local_operation(void);
  bool set_caps_that_disables_local_operation(unsigned int caps);
  unsigned int get_active_actions(void);
  _at_config_action_t get_action_assigned_to_capability(int cap);
  char get_percentage(int cap);
  _at_config_rgbw_t get_rgbw(int cap);
  bool channel_exists(int channel_id);
};

#endif /* ACTIONTRIGGERCONFIG_H_ */
