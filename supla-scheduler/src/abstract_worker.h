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

#ifndef ABSTRACT_WORKER_H_
#define ABSTRACT_WORKER_H_

#include "database.h"
#include "ipcclient.h"
#include "queue.h"

class s_abstract_worker {
 private:
  database *db;
  ipc_client *ipcc;
  queue *q;
  s_exec_params_t s_exec_params;

 protected:
  ipc_client *get_ipcc(void);
  queue *get_queue(void);
  const s_exec_params_t *get_params(void);

 public:
  explicit s_abstract_worker(queue *q);
  virtual ~s_abstract_worker();
  void execute(void *sthread);
  database *get_db(void);

  virtual int get_channel_func(void) = 0;
  virtual int get_id(void) = 0;
  virtual bool it_applies_to_scene(void) = 0;
  virtual int get_retry_count(void) = 0;
  virtual bool channel_group(void) = 0;
  virtual const char *get_action_param(void) = 0;
  virtual bool retry_when_fail(void) = 0;

  virtual char ipcc_get_opening_sensor_value(void) = 0;
  virtual bool ipcc_set_char_value(char value) = 0;
  virtual bool ipcc_get_char_value(char *value) = 0;
  virtual bool ipcc_get_rgbw_value(int *color, char *color_brightness,
                                   char *brightness) = 0;
  virtual bool ipcc_set_rgbw_value(int color, char color_brightness,
                                   char brightness) = 0;
  virtual bool ipcc_get_valve_value(TValve_Value *value) = 0;
  virtual bool ipcc_set_digiglass_value(int active_bits, int mask) = 0;
  virtual bool ipcc_get_digiglass_value(int *mask) = 0;
  virtual bool ipcc_action_copy(int sourceDeviceId, int sourceChannelId) = 0;
  virtual bool ipcc_execute_scene(void) = 0;
  virtual bool ipcc_interrupt_scene(void) = 0;
  virtual bool ipcc_interrupt_and_execute_scene(void) = 0;
  virtual char ipcc_is_connected(void) = 0;
};

#endif /* ABSTRACT_WORKER_H_ */
