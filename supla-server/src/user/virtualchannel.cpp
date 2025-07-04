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

#include "virtualchannel.h"

#include "device/value/channel_value_factory.h"
#include "user/user.h"
#include "vbt/value_based_triggers.h"

supla_virtual_channel::supla_virtual_channel(void) {
  this->device_id = 0;
  this->channel_id = 0;
  this->type = 0;
  this->func = 0;
  value_valid_to = {};
  value = nullptr;
}

supla_virtual_channel::supla_virtual_channel(
    const supla_virtual_channel &channel) {
  this->device_id = 0;
  this->channel_id = 0;
  this->type = 0;
  this->func = 0;
  value_valid_to = {};
  value = nullptr;

  *this = channel;
}

supla_virtual_channel::supla_virtual_channel(
    supla_user *user, int device_id, int channel_id,
    const char raw_value[SUPLA_CHANNELVALUE_SIZE],
    const unsigned _supla_int_t validity_time_sec, int type, int func,
    int param1, int param2, int param3, int param4, const char *user_config,
    const char *properties) {
  this->device_id = device_id;
  this->channel_id = channel_id;
  this->type = type;
  this->func = func;
  value_valid_to = {};

  value = supla_abstract_channel_value_factory::new_value(raw_value, type, func,
                                                          user, param2, param3);

  if (value) {
    supla_json_config json_config(nullptr);

    json_config.set_properties(properties);
    json_config.set_user_config(user_config);

    value->apply_channel_properties(type, SUPLA_PROTO_VERSION, param1, param2,
                                    param3, param4, &json_config);
  }

  gettimeofday(&value_valid_to, nullptr);
  value_valid_to.tv_sec += validity_time_sec;
}

supla_virtual_channel::~supla_virtual_channel() {
  if (value) {
    delete value;
  }
}

int supla_virtual_channel::get_device_id(void) { return device_id; }

int supla_virtual_channel::get_channel_id(void) { return channel_id; }

int supla_virtual_channel::get_func(void) { return func; }

int supla_virtual_channel::get_type(void) { return type; }

const supla_abstract_channel_value *supla_virtual_channel::get_value(void) {
  return value;
}

supla_channel_availability_status
supla_virtual_channel::get_availability_status(void) {
  supla_channel_availability_status result(true);

  struct timeval now;
  gettimeofday(&now, nullptr);

  if ((value_valid_to.tv_sec * 1000000LL + value_valid_to.tv_usec) -
          (now.tv_sec * 1000000LL + now.tv_usec) >
      0) {
    result.set_offline(false);
  }

  return result;
}

bool supla_virtual_channel::is_value_differ(
    supla_user *user, const supla_virtual_channel *channel, bool raise_events) {
  bool result = false;

  if (channel->channel_id != channel_id || !channel) {
    return result;
  }

  bool significant_change = false;
  if (func == channel->func && value && channel->value &&
      value->is_differ(channel->value, &significant_change)) {
    if (user && raise_events) {
      user->on_channel_value_changed(supla_caller(ctChannel, channel_id),
                                     device_id, channel_id, false,
                                     significant_change);

      user->get_value_based_triggers()->on_value_changed(
          supla_caller(ctChannel, channel_id), channel_id, value,
          channel->value);
    }
    result = true;
  }

  return result;
}

// static
unsigned _supla_int_t supla_virtual_channel::get_value_validity_time_sec(
    const struct timeval &value_valid_to) {
  unsigned _supla_int_t result = 0;

  if (value_valid_to.tv_sec > 0 || value_valid_to.tv_usec) {
    struct timeval now;
    gettimeofday(&now, nullptr);

    if (now.tv_sec < value_valid_to.tv_sec) {
      result = value_valid_to.tv_sec - now.tv_sec;
    }
  }

  return result;
}

unsigned supla_virtual_channel::get_value_validity_time_sec(void) {
  return get_value_validity_time_sec(value_valid_to);
}

bool supla_virtual_channel::get_value(char raw_value[SUPLA_CHANNELVALUE_SIZE]) {
  if (value) {
    value->get_raw_value(raw_value);
    return true;
  }

  return false;
}

supla_virtual_channel &supla_virtual_channel::operator=(
    const supla_virtual_channel &channel) {
  device_id = channel.device_id;
  channel_id = channel.channel_id;
  type = channel.type;
  func = channel.func;
  value_valid_to = channel.value_valid_to;

  if (value) {
    delete value;
  }

  if (channel.value) {
    value = channel.value->copy();
  }

  return *this;
}
