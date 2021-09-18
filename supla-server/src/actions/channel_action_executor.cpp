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

#include <actions/channel_action_executor.h>

supla_channel_action_executor::supla_channel_action_executor(void)
    : supla_abstract_channel_action_executor() {}

supla_channel_action_executor::supla_channel_action_executor(supla_user *user, int device_id,
                                             int channel_id)
    : supla_abstract_channel_action_executor(user, device_id, channel_id) {}

supla_channel_action_executor::supla_channel_action_executor(int user_id, int device_id,
                                             int channel_id)
    : supla_abstract_channel_action_executor(user_id, device_id, channel_id) {}

void supla_channel_action_executor::set_on(bool on) {
  supla_device *device = get_device();
  if (device) {
    device->get_channels()->set_on(0, get_channel_id(), 0, 0, on ? 1 : 0);
    device->releasePtr();
  }
}

void supla_channel_action_executor::set_color(unsigned int color) {
  supla_device *device = get_device();
  if (device) {
    device->get_channels()->set_color(0, get_channel_id(), 0, 0, color);
    device->releasePtr();
  }
}

void supla_channel_action_executor::set_brightness(char brightness) {
  supla_device *device = get_device();
  if (device) {
    device->get_channels()->set_brightness(0, get_channel_id(), 0, 0,
                                           brightness);
    device->releasePtr();
  }
}

void supla_channel_action_executor::set_color_brightness(char brightness) {
  supla_device *device = get_device();
  if (device) {
    device->get_channels()->set_color_brightness(0, get_channel_id(), 0, 0,
                                                 brightness);
    device->releasePtr();
  }
}

void supla_channel_action_executor::toggle(void) {
  supla_device *device = get_device();
  if (device) {
    device->get_channels()->action_toggle(0, get_channel_id(), 0, 0);
    device->releasePtr();
  }
}

void supla_channel_action_executor::shut(const char *closingPercentage) {
  supla_device *device = get_device();
  if (device) {
    device->get_channels()->action_shut(0, get_channel_id(), 0, 0,
                                        closingPercentage);
    device->releasePtr();
  }
}

void supla_channel_action_executor::reveal(void) {
  supla_device *device = get_device();
  if (device) {
    device->get_channels()->action_reveal(0, get_channel_id(), 0, 0);
    device->releasePtr();
  }
}

void supla_channel_action_executor::stop(void) {
  supla_device *device = get_device();
  if (device) {
    device->get_channels()->action_stop(0, get_channel_id(), 0, 0);
    device->releasePtr();
  }
}

void supla_channel_action_executor::open(void) {
  supla_device *device = get_device();
  if (device) {
    device->get_channels()->action_open(0, get_channel_id(), 0, 0);
    device->releasePtr();
  }
}

void supla_channel_action_executor::close(void) {
  supla_device *device = get_device();
  if (device) {
    device->get_channels()->action_close(get_channel_id());
    device->releasePtr();
  }
}

void supla_channel_action_executor::open_close(void) {
  supla_device *device = get_device();
  if (device) {
    device->get_channels()->action_open_close(0, get_channel_id(), 0, 0);
    device->releasePtr();
  }
}

void supla_channel_action_executor::open_close_without_canceling_tasks() {
  supla_device *device = get_device();
  if (device) {
    device->get_channels()->action_open_close_without_canceling_tasks(
        0, get_channel_id(), 0, 0);
    device->releasePtr();
  }
}
