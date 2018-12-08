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

#ifndef WORKER_H_
#define WORKER_H_

#include "database.h"
#include "ipcclient.h"

class queue;
class s_worker {
 private:
  s_exec_t s_exec;
  database *db;
  ipc_client *ipcc;
  queue *q;

 public:
  explicit s_worker(queue *q);
  ~s_worker();
  void execute(void *sthread);
  database *get_db(void);

  int get_channel_func(void);
  int get_id(void);
  int get_retry_count(void);
  bool channel_group(void);
  const char *get_action_param(void);
  bool retry_when_fail(void);

  char ipcc_get_opening_sensor_value(void);
  bool ipcc_set_char_value(char value);
  bool ipcc_get_char_value(char *value);
  bool ipcc_get_rgbw_value(int *color, char *color_brightness,
                           char *brightness);
  bool ipcc_set_rgbw_value(int color, char color_brightness, char brightness);
  char ipcc_is_connected(void);
};

#endif /* QUEUE_H_ */
