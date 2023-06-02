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

#ifndef SUPLA_CLIENT_SCENE_H_
#define SUPLA_CLIENT_SCENE_H_

#include "distributedobjects/dobject.h"
#include "proto.h"
#include "scene/scene_state.h"

class supla_client_scene : public supla_dobject {
 private:
  char *caption;
  int user_icon_id;
  int alt_icon_id;
  int location_id;
  supla_scene_state state;
  void mark_as_changed(void);

 public:
  explicit supla_client_scene(int id);
  virtual ~supla_client_scene();
  void set_caption(const char *caption);
  const char *get_caption(void);
  void set_user_icon_id(int user_icon_id);
  int get_user_icon_id(void);
  void set_alt_icon_id(int alt_icon_id);
  int get_alt_icon_id(void);
  void set_location_id(int location_id);
  int get_location_id(void);
  const supla_scene_state &get_state(void);
  void set_state(const supla_scene_state &state);

  void convert(TSC_SuplaScene *dest);
  void convert(TSC_SuplaSceneState *dest);
};

#endif /* SUPLA_CLIENT_SCENE_H_ */
