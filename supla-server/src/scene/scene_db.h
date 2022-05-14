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

#ifndef SCENE_DB_H_
#define SCENE_DB_H_

#include <list>

#include "scene/scene_operations.h"
#include "svrdb.h"

#define SCENE_CAPTION_MAXSIZE 256
#define SCENE_ACTION_PARAM_MAXSIZE 256

typedef struct {
  int scene_id;
  char caption[SCENE_CAPTION_MAXSIZE];
  int location_id;
} _supla_db_scene_row_t;

typedef struct {
  int owning_scene_id;
  int channel_id;
  int channel_group_id;
  int scene_id;
  int action_id;
  char action_param[SCENE_ACTION_PARAM_MAXSIZE];
  int delay_ms;
} _supla_db_operation_row_t;

class supla_scene_db : public svrdb {
 private:
  supla_scene_operation *convert(_supla_db_operation_row_t *row);

 public:
  supla_scene_db(void);
  virtual ~supla_scene_db(void);

  std::list<supla_scene_operation *> get_operations(int scene_id);
};

#endif /*SCENE_DB_H_*/
