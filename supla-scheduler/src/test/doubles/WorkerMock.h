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

#include <gmock/gmock.h>

#include "abstract_worker.h"

class WorkerMock : public s_abstract_worker {
 public:
  explicit WorkerMock(queue *q);
  virtual ~WorkerMock();

  MOCK_METHOD0(get_channel_func, int(void));
  MOCK_METHOD0(get_id, int(void));
  MOCK_METHOD0(it_applies_to_scene, bool(void));
  MOCK_METHOD0(get_retry_count, int(void));
  MOCK_METHOD0(channel_group, bool(void));
  MOCK_METHOD0(get_action_param, const char *(void));
  MOCK_METHOD0(retry_when_fail, bool(void));
  MOCK_METHOD0(ipcc_get_opening_sensor_value, char(void));
  MOCK_METHOD1(ipcc_set_char_value, bool(char value));
  MOCK_METHOD1(ipcc_get_char_value, bool(char *value));
  MOCK_METHOD3(ipcc_get_rgbw_value,
               bool(int *color, char *color_brightness, char *brightness));
  MOCK_METHOD3(ipcc_set_rgbw_value,
               bool(int color, char color_brightness, char brightness));
  MOCK_METHOD1(ipcc_get_valve_value, bool(TValve_Value *value));
  MOCK_METHOD2(ipcc_set_digiglass_value, bool(int active_bits, int mask));
  MOCK_METHOD1(ipcc_get_digiglass_value, bool(int *mask));
  MOCK_METHOD2(ipcc_action_copy, bool(int sourceDeviceId, int sourceChannelId));
  MOCK_METHOD0(ipcc_execute_scene, bool(void));
  MOCK_METHOD0(ipcc_interrupt_scene, bool(void));
  MOCK_METHOD0(ipcc_is_connected, char(void));
};

#endif /*WORKERMOCK_TEST_H_*/
