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

#include "actions/channel_action_executor.h"

#include "userchannelgroups.h"

supla_channel_action_executor::supla_channel_action_executor(void)
    : supla_abstract_channel_action_executor() {}

supla_channel_action_executor::supla_channel_action_executor(supla_user *user,
                                                             int device_id,
                                                             int channel_id)
    : supla_abstract_channel_action_executor(user, device_id, channel_id) {}

supla_channel_action_executor::supla_channel_action_executor(int user_id,
                                                             int device_id,
                                                             int channel_id)
    : supla_abstract_channel_action_executor(user_id, device_id, channel_id) {}

void supla_channel_action_executor::set_on(bool on) {
  if (get_channel_groups()) {
    get_channel_groups()->set_on(get_group_id(), on);
  } else {
    supla_device *device = get_device();
    if (device) {
      device->get_channels()->set_on(0, get_channel_id(), 0, 0, on ? 1 : 0);
      device->releasePtr();
    }
  }
}

void supla_channel_action_executor::set_color(unsigned int color) {
  if (get_channel_groups()) {
    get_channel_groups()->set_color(get_group_id(), color);
  } else {
    supla_device *device = get_device();
    if (device) {
      device->get_channels()->set_color(0, get_channel_id(), 0, 0, color);
      device->releasePtr();
    }
  }
}

void supla_channel_action_executor::set_brightness(char brightness) {
  if (get_channel_groups()) {
    get_channel_groups()->set_brightness(get_group_id(), brightness);
  } else {
    supla_device *device = get_device();
    if (device) {
      device->get_channels()->set_brightness(0, get_channel_id(), 0, 0,
                                             brightness);
      device->releasePtr();
    }
  }
}

void supla_channel_action_executor::set_color_brightness(
    char color_brightness) {
  if (get_channel_groups()) {
    get_channel_groups()->set_color_brightness(get_group_id(),
                                               color_brightness);
  } else {
    supla_device *device = get_device();
    if (device) {
      device->get_channels()->set_color_brightness(0, get_channel_id(), 0, 0,
                                                   color_brightness);
      device->releasePtr();
    }
  }
}

void supla_channel_action_executor::set_rgbw(unsigned int *color,
                                             char *color_brightness,
                                             char *brightness) {
  if (get_channel_groups()) {
    get_channel_groups()->set_rgbw_value(get_group_id(), color,
                                         color_brightness, brightness, NULL);
  } else {
    supla_device *device = get_device();
    if (device) {
      device->get_channels()->set_rgbw(0, get_channel_id(), 0, 0, color,
                                       color_brightness, brightness, NULL);
      device->releasePtr();
    }
  }
}

void supla_channel_action_executor::toggle(void) {
  if (get_channel_groups()) {
    get_channel_groups()->action_toggle(get_group_id());
  } else {
    supla_device *device = get_device();
    if (device) {
      device->get_channels()->action_toggle(0, get_channel_id(), 0, 0);
      device->releasePtr();
    }
  }
}

void supla_channel_action_executor::shut(const char *closingPercentage) {
  if (get_channel_groups()) {
    get_channel_groups()->action_shut(get_group_id(), closingPercentage);
  } else {
    supla_device *device = get_device();
    if (device) {
      device->get_channels()->action_shut(0, get_channel_id(), 0, 0,
                                          closingPercentage);
      device->releasePtr();
    }
  }
}

void supla_channel_action_executor::reveal(void) {
  if (get_channel_groups()) {
    get_channel_groups()->action_reveal(get_group_id());
  } else {
    supla_device *device = get_device();
    if (device) {
      device->get_channels()->action_reveal(0, get_channel_id(), 0, 0);
      device->releasePtr();
    }
  }
}

void supla_channel_action_executor::stop(void) {
  if (get_channel_groups()) {
    get_channel_groups()->action_stop(get_group_id());
  } else {
    supla_device *device = get_device();
    if (device) {
      device->get_channels()->action_stop(0, get_channel_id(), 0, 0);
      device->releasePtr();
    }
  }
}

void supla_channel_action_executor::open(void) {
  if (get_channel_groups()) {
    get_channel_groups()->action_open(get_group_id());
  } else {
    supla_device *device = get_device();
    if (device) {
      device->get_channels()->action_open(0, get_channel_id(), 0, 0);
      device->releasePtr();
    }
  }
}

void supla_channel_action_executor::close(void) {
  if (get_channel_groups()) {
    get_channel_groups()->action_close(get_group_id());
  } else {
    supla_device *device = get_device();
    if (device) {
      device->get_channels()->action_close(get_channel_id());
      device->releasePtr();
    }
  }
}

void supla_channel_action_executor::open_close(void) {
  if (get_channel_groups()) {
    get_channel_groups()->action_open_close(get_group_id());
  } else {
    supla_device *device = get_device();
    if (device) {
      device->get_channels()->action_open_close(0, get_channel_id(), 0, 0);
      device->releasePtr();
    }
  }
}

void supla_channel_action_executor::open_close_without_canceling_tasks() {
  if (get_channel_groups()) {
  } else {
    supla_device *device = get_device();
    if (device) {
      device->get_channels()->action_open_close_without_canceling_tasks(
          0, get_channel_id(), 0, 0);
      device->releasePtr();
    }
  }
}
