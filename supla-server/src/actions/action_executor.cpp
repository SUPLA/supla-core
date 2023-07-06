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

#include "actions/action_executor.h"

#include <memory>

#include "http/http_event_hub.h"
#include "mqtt/mqtt_client_suite.h"
#include "push/pn_delivery_task.h"
#include "scene/scene_asynctask.h"
#include "schedule/schedule_dao.h"
#include "userchannelgroups.h"

using std::map;
using std::shared_ptr;
using std::string;

supla_action_executor::supla_action_executor(void)
    : supla_abstract_action_executor() {}

void supla_action_executor::set_on(bool on) {
  execute_action([this, on](supla_user_channelgroups *channel_groups,
                            supla_device_channels *channels) -> void {
    if (channel_groups) {
      channel_groups->set_on(get_caller(), get_group_id(), on);
    } else {
      channels->set_on(get_caller(), get_channel_id(), 0, 0, on ? 1 : 0);
    }
  });
}

void supla_action_executor::set_color(unsigned int color) {
  execute_action([this, color](supla_user_channelgroups *channel_groups,
                               supla_device_channels *channels) -> void {
    if (channel_groups) {
      channel_groups->set_color(get_caller(), get_group_id(), color);
    } else {
      channels->set_color(get_caller(), get_channel_id(), 0, 0, color);
    }
  });
}

void supla_action_executor::set_brightness(char brightness) {
  execute_action([this, brightness](supla_user_channelgroups *channel_groups,
                                    supla_device_channels *channels) -> void {
    if (channel_groups) {
      channel_groups->set_brightness(get_caller(), get_group_id(), brightness);
    } else {
      channels->set_brightness(get_caller(), get_channel_id(), 0, 0,
                               brightness);
    }
  });
}

void supla_action_executor::set_color_brightness(char color_brightness) {
  execute_action(
      [this, color_brightness](supla_user_channelgroups *channel_groups,
                               supla_device_channels *channels) -> void {
        if (channel_groups) {
          channel_groups->set_color_brightness(get_caller(), get_group_id(),
                                               color_brightness);
        } else {
          channels->set_color_brightness(get_caller(), get_channel_id(), 0, 0,
                                         color_brightness);
        }
      });
}

void supla_action_executor::set_rgbw(unsigned int *color,
                                     char *color_brightness, char *brightness,
                                     char *on_off) {
  execute_action([this, color, color_brightness, brightness, on_off](
                     supla_user_channelgroups *channel_groups,
                     supla_device_channels *channels) -> void {
    if (channel_groups) {
      channel_groups->set_rgbw_value(get_caller(), get_group_id(), color,
                                     color_brightness, brightness, on_off);
    } else {
      channels->set_rgbw(get_caller(), get_channel_id(), 0, 0, color,
                         color_brightness, brightness, on_off);
    }
  });
}

void supla_action_executor::toggle(void) {
  execute_action([this](supla_user_channelgroups *channel_groups,
                        supla_device_channels *channels) -> void {
    if (channel_groups) {
      channel_groups->action_toggle(get_caller(), get_group_id());
    } else {
      channels->action_toggle(get_caller(), get_channel_id(), 0, 0);
    }
  });
}

void supla_action_executor::shut(const char *closingPercentage, bool delta) {
  execute_action([this, closingPercentage, delta](
                     supla_user_channelgroups *channel_groups,
                     supla_device_channels *channels) -> void {
    if (channel_groups) {
      channel_groups->action_shut(get_caller(), get_group_id(),
                                  closingPercentage, delta);
    } else {
      channels->action_shut(get_caller(), get_channel_id(), 0, 0,
                            closingPercentage, delta);
    }
  });
}

void supla_action_executor::reveal(void) {
  execute_action([this](supla_user_channelgroups *channel_groups,
                        supla_device_channels *channels) -> void {
    if (channel_groups) {
      channel_groups->action_reveal(get_caller(), get_group_id());
    } else {
      channels->action_reveal(get_caller(), get_channel_id(), 0, 0);
    }
  });
}

void supla_action_executor::execute(void) {
  if (get_scene_id() && get_user()) {
    supla_scene_asynctask::execute(
        supla_scene_asynctask::get_queue(), supla_scene_asynctask::get_pool(),
        get_caller(), get_user()->getUserID(), get_scene_id(), false);
  }
}

void supla_action_executor::interrupt(void) {
  if (get_scene_id() && get_user()) {
    supla_scene_asynctask::interrupt(supla_scene_asynctask::get_queue(),
                                     get_user()->getUserID(), get_scene_id());
  }
}

void supla_action_executor::interrupt_and_execute(void) {
  if (get_scene_id() && get_user()) {
    supla_scene_asynctask::execute(
        supla_scene_asynctask::get_queue(), supla_scene_asynctask::get_pool(),
        get_caller(), get_user()->getUserID(), get_scene_id(), true);
  }
}

void supla_action_executor::enable(void) {
  supla_db_access_provider dba;
  supla_schedule_dao dao(&dba);
  dao.enable(get_user_id(), get_schedule_id(), true);
}

void supla_action_executor::disable(void) {
  supla_db_access_provider dba;
  supla_schedule_dao dao(&dba);
  dao.enable(get_user_id(), get_schedule_id(), false);
}

void supla_action_executor::send(map<string, string> *replacement_map) {
  if (get_user()) {
    supla_pn_delivery_task::start_delivering(
        get_user()->getUserID(), get_push_notification_id(), replacement_map);
  }
}

void supla_action_executor::stop(void) {
  execute_action([this](supla_user_channelgroups *channel_groups,
                        supla_device_channels *channels) -> void {
    if (channel_groups) {
      channel_groups->action_stop(get_caller(), get_group_id());
    } else {
      channels->action_stop(get_caller(), get_channel_id(), 0, 0);
    }
  });
}

void supla_action_executor::up(void) {
  execute_action([this](supla_user_channelgroups *channel_groups,
                        supla_device_channels *channels) -> void {
    if (channel_groups) {
      channel_groups->action_up(get_caller(), get_group_id());
    } else {
      channels->action_up(get_caller(), get_channel_id(), 0, 0);
    }
  });
}

void supla_action_executor::down(void) {
  execute_action([this](supla_user_channelgroups *channel_groups,
                        supla_device_channels *channels) -> void {
    if (channel_groups) {
      channel_groups->action_down(get_caller(), get_group_id());
    } else {
      channels->action_down(get_caller(), get_channel_id(), 0, 0);
    }
  });
}

void supla_action_executor::up_or_stop(void) {
  execute_action([this](supla_user_channelgroups *channel_groups,
                        supla_device_channels *channels) -> void {
    if (channel_groups) {
      channel_groups->action_up_or_stop(get_caller(), get_group_id());
    } else {
      channels->action_up_or_stop(get_caller(), get_channel_id(), 0, 0);
    }
  });
}

void supla_action_executor::down_or_stop(void) {
  execute_action([this](supla_user_channelgroups *channel_groups,
                        supla_device_channels *channels) -> void {
    if (channel_groups) {
      channel_groups->action_down_or_stop(get_caller(), get_group_id());
    } else {
      channels->action_down_or_stop(get_caller(), get_channel_id(), 0, 0);
    }
  });
}

void supla_action_executor::step_by_step(void) {
  execute_action([this](supla_user_channelgroups *channel_groups,
                        supla_device_channels *channels) -> void {
    if (channel_groups) {
      channel_groups->action_step_by_step(get_caller(), get_group_id());
    } else {
      channels->action_step_by_step(get_caller(), get_channel_id(), 0, 0);
    }
  });
}

void supla_action_executor::open(void) {
  execute_action([this](supla_user_channelgroups *channel_groups,
                        supla_device_channels *channels) -> void {
    if (channel_groups) {
      channel_groups->action_open(get_caller(), get_group_id());
    } else {
      channels->action_open(get_caller(), get_channel_id(), 0, 0);
    }
  });
}

void supla_action_executor::close(void) {
  execute_action([this](supla_user_channelgroups *channel_groups,
                        supla_device_channels *channels) -> void {
    if (channel_groups) {
      channel_groups->action_close(get_caller(), get_group_id());
    } else {
      channels->action_close(get_caller(), get_channel_id());
    }
  });
}

void supla_action_executor::open_close(void) {
  execute_action([this](supla_user_channelgroups *channel_groups,
                        supla_device_channels *channels) -> void {
    if (channel_groups) {
      channel_groups->action_open_close(get_caller(), get_group_id());
    } else {
      channels->action_open_close(get_caller(), get_channel_id(), 0, 0);
    }
  });
}

void supla_action_executor::open_close_without_canceling_tasks() {
  execute_action([this](supla_user_channelgroups *channel_groups,
                        supla_device_channels *channels) -> void {
    if (!channel_groups) {
      channels->action_open_close_without_canceling_tasks(
          get_caller(), get_channel_id(), 0, 0);
    }
  });
}

void supla_action_executor::forward_outside(int cap) {
  // device_id can be set to 0 so better to use device-> getID () inside the
  // access_device method.

  shared_ptr<supla_device> device = get_device();
  if (device != nullptr) {
    supla_mqtt_client_suite::globalInstance()->onActionsTriggered(
        device->get_user_id(), device->get_id(), get_channel_id(), cap);

    supla_http_event_hub::on_actions_triggered(get_caller(), device->get_user(),
                                               device->get_id(),
                                               get_channel_id(), cap);
  }
}
