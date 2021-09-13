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

#include "channeljsonconfig.h"

class action_trigger_config : public channel_json_config {
 private:
  bool equal_to_string(cJSON *item, const char *str);
  void add_cap(unsigned int caps, unsigned int cap, const char *name,
               cJSON *arr);

 public:
  action_trigger_config(channel_json_config *root);
  action_trigger_config(void);

  unsigned int get_capabilities(void);
  bool set_capabilities(unsigned int caps);
  unsigned int get_active_actions(void);
};

#endif /* ACTIONTRIGGERCONFIG_H_ */
