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

#include "asynctask/asynctask_state.h"

supla_asynctask_state::supla_asynctask_state() { state = INIT; }

supla_asynctask_state::~supla_asynctask_state(void) {}

void supla_asynctask_state::set_state(_state_e state) { this->state = state; }

supla_asynctask_state::_state_e supla_asynctask_state::get_state(void) const {
  return state;
}

supla_asynctask_state &supla_asynctask_state::operator=(const _state_e state) {
  this->state = state;
  return *this;
}

supla_asynctask_state::operator _state_e() { return get_state(); }

bool supla_asynctask_state::operator==(const _state_e state) const {
  return this->state == state;
}

bool supla_asynctask_state::operator!=(const _state_e state) const {
  return this->state != state;
}

bool supla_asynctask_state::operator==(
    const supla_asynctask_state &state) const {
  return this->state == state.get_state();
}

bool supla_asynctask_state::operator!=(
    const supla_asynctask_state &state) const {
  return this->state != state.get_state();
}
