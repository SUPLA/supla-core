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

#ifndef WORKERMOCK_TEST_H_
#define WORKERMOCK_TEST_H_

#include "abstract_worker.h"

class WorkerMock : public s_abstract_worker {
 private:
  char *action_param;

 public:
  explicit WorkerMock(queue *q);
  virtual ~WorkerMock();
  virtual int get_channel_func(void);
  virtual int get_id(void);
  virtual int get_retry_count(void);
  virtual bool channel_group(void);
  virtual const char *get_action_param(void);
  void set_action_param(const char *action_param);
  virtual bool retry_when_fail(void);

  virtual char ipcc_get_opening_sensor_value(void);
  virtual bool ipcc_set_char_value(char value);
  virtual bool ipcc_get_char_value(char *value);
  virtual bool ipcc_get_rgbw_value(int *color, char *color_brightness,
                                   char *brightness);
  virtual bool ipcc_set_rgbw_value(int color, char color_brightness,
                                   char brightness);
  virtual bool ipcc_get_valve_value(TValve_Value *value);
  virtual bool ipcc_set_digiglass_value(int active_bits, int mask);
  virtual bool ipcc_get_digiglass_value(int *mask);
  virtual char ipcc_is_connected(void);
};

#endif /*WORKERMOCK_TEST_H_*/
