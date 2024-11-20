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

#include "channel_fragment.h"

#include "device/device.h"

supla_channel_fragment::supla_channel_fragment() {
  device_id = 0;
  channel_id = 0;
  channel_number = 0;
  type = 0;
  function = 0;
  flags = 0;
  _is_hidden = true;
}

supla_channel_fragment::supla_channel_fragment(int device_id, int channel_id,
                                               unsigned char channel_number,
                                               int type, int function,
											   unsigned _supla_int64_t flags,
                                               bool _is_hidden) {
  this->device_id = device_id;
  this->channel_id = channel_id;
  this->channel_number = channel_number;
  this->type = type;
  this->function = function;
  this->flags = flags;
  this->_is_hidden = _is_hidden;
}

int supla_channel_fragment::get_device_id(void) { return device_id; }

int supla_channel_fragment::get_channel_id(void) { return channel_id; }

unsigned char supla_channel_fragment::get_channel_number(void) {
  return channel_number;
}

int supla_channel_fragment::get_type(void) { return type; }

int supla_channel_fragment::get_function(void) { return function; }

unsigned _supla_int64_t supla_channel_fragment::get_flags(void) { return flags; }

void supla_channel_fragment::set_function(int function) {
  this->function = function;
}

bool supla_channel_fragment::is_hidden(void) { return _is_hidden; }

supla_channel_fragment &supla_channel_fragment::operator=(
    supla_device_channel *channel) {
  if (channel) {
    device_id =
        channel->get_device() != nullptr ? channel->get_device()->get_id() : 0;
    channel_id = channel->get_id();
    channel_number = channel->get_channel_number();
    type = channel->get_type();
    function = channel->get_func();
    flags = channel->get_flags();
    _is_hidden = channel->is_hidden();
  } else {
    device_id = 0;
    channel_id = 0;
    channel_number = 0;
    type = 0;
    function = 0;
    flags = 0;
    _is_hidden = false;
  }
  return *this;
}
