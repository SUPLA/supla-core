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

#ifndef SUPLA_SCENES_H_
#define SUPLA_SCENES_H_

#include "distributedobjects/dobjects.h"
#include "scene_repository_factory.h"

class supla_scenes : public supla_dobjects {
 private:
 protected:
 public:
  explicit supla_scenes(supla_scene_repository_factory* repository_factory);
  virtual ~supla_scenes(void);
};

#endif /* SUPLA_SCENES_H_ */
