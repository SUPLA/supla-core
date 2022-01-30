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

#include "http/httprequestqueue.h"
#include "mqtt/mqtt_client_suite.h"
#include "userchannelgroups.h"

supla_action_executor::supla_action_executor(void)
    : supla_abstract_action_executor() {}

supla_action_executor::supla_action_executor(supla_user *user, int device_id,
                                             int channel_id)
    : supla_abstract_action_executor(user, device_id, channel_id) {}

supla_action_executor::supla_action_executor(int user_id, int device_id,
                                             int channel_id)
    : supla_abstract_action_executor(user_id, device_id, channel_id) {}

void supla_action_executor::execute_action(
    std::function<void(supla_user_channelgroups *, supla_device_channels *)>
        f) {
  supla_user_channelgroups *channel_groups = get_channel_groups();
  if (channel_groups) {
    f(channel_groups, NULL);
  } else {
    access_device([f](supla_device *device) -> void {
      supla_device_channels *channels = device->get_channels();
      if (channels) {
        f(NULL, channels);
      }
    });
  }
}

void supla_action_executor::set_on(bool on) {
  execute_action([this, on](supla_user_channelgroups *channel_groups,
                            supla_device_channels *channels) -> void {
    if (channel_groups) {
      channel_groups->set_on(get_group_id(), on);
    } else {
      channels->set_on(0, get_channel_id(), 0, 0, on ? 1 : 0);
    }
  });
}

void supla_action_executor::set_color(unsigned int color) {
  execute_action([this, color](supla_user_channelgroups *channel_groups,
                               supla_device_channels *channels) -> void {
    if (channel_groups) {
      channel_groups->set_color(get_group_id(), color);
    } else {
      channels->set_color(0, get_channel_id(), 0, 0, color);
    }
  });
}

void supla_action_executor::set_brightness(char brightness) {
  execute_action([this, brightness](supla_user_channelgroups *channel_groups,
                                    supla_device_channels *channels) -> void {
    if (channel_groups) {
      channel_groups->set_brightness(get_group_id(), brightness);
    } else {
      channels->set_brightness(0, get_channel_id(), 0, 0, brightness);
    }
  });
}

void supla_action_executor::set_color_brightness(char color_brightness) {
  execute_action([this, color_brightness](
                     supla_user_channelgroups *channel_groups,
                     supla_device_channels *channels) -> void {
    if (channel_groups) {
      channel_groups->set_color_brightness(get_group_id(), color_brightness);
    } else {
      channels->set_color_brightness(0, get_channel_id(), 0, 0,
                                     color_brightness);
    }
  });
}

void supla_action_executor::set_rgbw(unsigned int *color,
                                     char *color_brightness, char *brightness) {
  execute_action([this, color, color_brightness, brightness](
                     supla_user_channelgroups *channel_groups,
                     supla_device_channels *channels) -> void {
    if (channel_groups) {
      channel_groups->set_rgbw_value(get_group_id(), color, color_brightness,
                                     brightness, NULL);
    } else {
      channels->set_rgbw(0, get_channel_id(), 0, 0, color, color_brightness,
                         brightness, NULL);
    }
  });
}

void supla_action_executor::toggle(void) {
  execute_action([this](supla_user_channelgroups *channel_groups,
                        supla_device_channels *channels) -> void {
    if (channel_groups) {
      channel_groups->action_toggle(get_group_id());
    } else {
      channels->action_toggle(0, get_channel_id(), 0, 0);
    }
  });
}

void supla_action_executor::shut(const char *closingPercentage) {
  execute_action(
      [this, closingPercentage](supla_user_channelgroups *channel_groups,
                                supla_device_channels *channels) -> void {
        if (channel_groups) {
          channel_groups->action_shut(get_group_id(), closingPercentage);
        } else {
          channels->action_shut(0, get_channel_id(), 0, 0, closingPercentage);
        }
      });
}

void supla_action_executor::reveal(void) {
  execute_action([this](supla_user_channelgroups *channel_groups,
                        supla_device_channels *channels) -> void {
    if (channel_groups) {
      channel_groups->action_reveal(get_group_id());
    } else {
      channels->action_reveal(0, get_channel_id(), 0, 0);
    }
  });
}

void supla_action_executor::stop(void) {
  execute_action([this](supla_user_channelgroups *channel_groups,
                        supla_device_channels *channels) -> void {
    if (channel_groups) {
      channel_groups->action_stop(get_group_id());
    } else {
      channels->action_stop(0, get_channel_id(), 0, 0);
    }
  });
}

void supla_action_executor::up(void) {
  execute_action([this](supla_user_channelgroups *channel_groups,
                        supla_device_channels *channels) -> void {
    if (channel_groups) {
      channel_groups->action_up(get_group_id());
    } else {
      channels->action_up(0, get_channel_id(), 0, 0);
    }
  });
}

void supla_action_executor::down(void) {
  execute_action([this](supla_user_channelgroups *channel_groups,
                        supla_device_channels *channels) -> void {
    if (channel_groups) {
      channel_groups->action_down(get_group_id());
    } else {
      channels->action_down(0, get_channel_id(), 0, 0);
    }
  });
}

void supla_action_executor::up_or_stop(void) {
  execute_action([this](supla_user_channelgroups *channel_groups,
                        supla_device_channels *channels) -> void {
    if (channel_groups) {
      channel_groups->action_up_or_stop(get_group_id());
    } else {
      channels->action_up_or_stop(0, get_channel_id(), 0, 0);
    }
  });
}

void supla_action_executor::down_or_stop(void) {
  execute_action([this](supla_user_channelgroups *channel_groups,
                        supla_device_channels *channels) -> void {
    if (channel_groups) {
      channel_groups->action_down_or_stop(get_group_id());
    } else {
      channels->action_down_or_stop(0, get_channel_id(), 0, 0);
    }
  });
}

void supla_action_executor::step_by_step(void) {
  execute_action([this](supla_user_channelgroups *channel_groups,
                        supla_device_channels *channels) -> void {
    if (channel_groups) {
      channel_groups->action_step_by_step(get_group_id());
    } else {
      channels->action_step_by_step(0, get_channel_id(), 0, 0);
    }
  });
}

void supla_action_executor::open(void) {
  execute_action([this](supla_user_channelgroups *channel_groups,
                        supla_device_channels *channels) -> void {
    if (channel_groups) {
      channel_groups->action_open(get_group_id());
    } else {
      channels->action_open(0, get_channel_id(), 0, 0);
    }
  });
}

void supla_action_executor::close(void) {
  execute_action([this](supla_user_channelgroups *channel_groups,
                        supla_device_channels *channels) -> void {
    if (channel_groups) {
      channel_groups->action_close(get_group_id());
    } else {
      channels->action_close(get_channel_id());
    }
  });
}

void supla_action_executor::open_close(void) {
  execute_action([this](supla_user_channelgroups *channel_groups,
                        supla_device_channels *channels) -> void {
    if (channel_groups) {
      channel_groups->action_open_close(get_group_id());
    } else {
      channels->action_open_close(0, get_channel_id(), 0, 0);
    }
  });
}

void supla_action_executor::open_close_without_canceling_tasks() {
  execute_action([this](supla_user_channelgroups *channel_groups,
                        supla_device_channels *channels) -> void {
    if (!channel_groups) {
      channels->action_open_close_without_canceling_tasks(0, get_channel_id(),
                                                          0, 0);
    }
  });
}

void supla_action_executor::forward_outside(int cap) {
  // device_id can be set to 0 so better to use device-> getID () inside the
  // access_device method.
  access_device([this, cap](supla_device *device) -> void {
    supla_mqtt_client_suite::globalInstance()->onActionsTriggered(
        device->getUserID(), device->getID(), get_channel_id(), cap);

    supla_http_request_queue::getInstance()->onActionsTriggered(
        device->getUser(), device->getID(), get_channel_id(), cap);
  });
}
