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

#ifndef SUPLA_CLIENT_SCENE_CHANGE_INDICATOR_H_
#define SUPLA_CLIENT_SCENE_CHANGE_INDICATOR_H_

#include "distributedobjects/dobject_change_indicator.h"

class supla_client_scene_change_indicator
    : public supla_dobject_change_indicator {
 private:
  bool scene_changed;
  bool state_changed;

 public:
  supla_client_scene_change_indicator();
  explicit supla_client_scene_change_indicator(bool changed);
  supla_client_scene_change_indicator(bool scene_changed, bool state_changed);
  virtual ~supla_client_scene_change_indicator();

  bool is_scene_changed(void) const;
  bool is_state_changed(void) const;
};

#endif /* SUPLA_CLIENT_SCENE_CHANGE_INDICATOR_H_ */
