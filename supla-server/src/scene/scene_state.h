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

#ifndef SUPLA_SCENE_STATE_H_
#define SUPLA_SCENE_STATE_H_

#include "caller.h"
#include "proto.h"

class supla_scene_state {
 private:
  struct timeval started_at;
  struct timeval ending_at;
  supla_caller caller;

 public:
  supla_scene_state(void);
  supla_scene_state(const supla_scene_state &state);
  supla_scene_state(const supla_caller &caller, struct timeval started_at,
                    unsigned _supla_int_t millis_left);
  virtual ~supla_scene_state();

  const supla_caller &get_caller(void) const;
  bool is_during_execution(void) const;
  int get_initiator_id(void) const;
  const char *get_initiator_name(void) const;
  struct timeval get_started_at(void) const;
  struct timeval get_ending_at(void) const;
  unsigned int get_milliseconds_from_start(void) const;
  unsigned int get_milliseconds_left(void) const;
  void convert(TSC_SuplaSceneState *dest) const;

  supla_scene_state &operator=(const supla_scene_state &state);
  bool operator==(const supla_scene_state &state) const;
  bool operator!=(const supla_scene_state &state) const;
};

#endif /* SUPLA_SCENE_STATE_H_ */
