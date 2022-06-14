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

#include "scene/scene_state.h"

#include <stdlib.h>

#include "string.h"

supla_scene_state::supla_scene_state(void) {
  this->started_at = {};
  this->ending_at = {};
}

supla_scene_state::supla_scene_state(const supla_scene_state &state) {
  *this = state;
}

supla_scene_state::supla_scene_state(const supla_caller &caller,
                                     struct timeval started_at,
                                     unsigned _supla_int_t millis_left) {
  this->started_at = started_at;
  unsigned long long ending_at_usec =
      started_at.tv_sec * 1000000 + started_at.tv_usec + millis_left * 1000;
  this->ending_at.tv_sec = ending_at_usec / 1000000;
  this->ending_at.tv_usec = ending_at_usec % 1000000;
  this->caller = caller;
}

supla_scene_state::~supla_scene_state(void) {}

const supla_caller &supla_scene_state::get_caller(void) const { return caller; }

bool supla_scene_state::is_during_execution(void) const {
  bool result = false;

  if (started_at.tv_sec || started_at.tv_usec) {
    struct timeval now = {};
    gettimeofday(&now, NULL);

    result =
        started_at.tv_sec < now.tv_sec ||
        (started_at.tv_sec == now.tv_sec && started_at.tv_usec <= now.tv_usec);

    if (result) {
      result =
          ending_at.tv_sec > now.tv_sec ||
          (ending_at.tv_sec == now.tv_sec && ending_at.tv_usec > now.tv_usec);
    }
  }

  return result;
}

int supla_scene_state::get_initiator_id(void) const {
  return caller.get_type() == ctClient ? caller.get_id() : 0;
}

const char *supla_scene_state::get_initiator_name(void) const {
  return caller.get_type() == ctClient && caller.get_name().length()
             ? caller.get_name().c_str()
             : NULL;
}

struct timeval supla_scene_state::get_started_at(void) const {
  return started_at;
}

struct timeval supla_scene_state::get_ending_at(void) const {
  return ending_at;
}

unsigned _supla_int_t
supla_scene_state::get_milliseconds_from_start(void) const {
  if (started_at.tv_sec || started_at.tv_usec) {
    struct timeval now = {};
    gettimeofday(&now, NULL);

    unsigned long long now_usec = now.tv_sec * 1000000 + now.tv_usec;
    unsigned long long started_at_usec =
        started_at.tv_sec * 1000000 + started_at.tv_usec;

    if (now_usec >= started_at_usec) {
      return (now_usec - started_at_usec) / 1000;
    }
  }

  return 0;
}

unsigned _supla_int_t supla_scene_state::get_milliseconds_left(void) const {
  if (ending_at.tv_sec || ending_at.tv_usec) {
    struct timeval now = {};
    gettimeofday(&now, NULL);

    unsigned long long now_usec = now.tv_sec * 1000000 + now.tv_usec;
    unsigned long long ending_at_usec =
        ending_at.tv_sec * 1000000 + ending_at.tv_usec;

    if (now_usec < ending_at_usec) {
      return (ending_at_usec - now_usec) / 1000;
    }
  }

  return 0;
}

void supla_scene_state::convert(TSC_SuplaSceneState *dest) const {
  *dest = {};

  dest->MillisecondsFromStart = get_milliseconds_from_start();
  dest->MillisecondsLeft = get_milliseconds_left();

  dest->InitiatorId = get_initiator_id();
  sproto__set_null_terminated_string(get_initiator_name(), dest->InitiatorName,
                                     &dest->InitiatorNameSize,
                                     SUPLA_INITIATOR_NAME_MAXSIZE);
}

supla_scene_state &supla_scene_state::operator=(
    const supla_scene_state &state) {
  started_at = state.started_at;
  ending_at = state.ending_at;
  caller = state.caller;
  return *this;
}
