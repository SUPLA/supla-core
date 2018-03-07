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

#ifndef SUPLA_SCHEDULER_H_
#define SUPLA_SCHEDULER_H_

typedef struct {
  int id;
  int type;
  int func;
  int param1;
  int param2;
  int param3;
  int iodevice_id;
} supla_channel;

typedef struct {
  int id;
  int schedule_id;
  int user_id;
  int iodevice_id;
  int channel_id;
  int channel_func;
  int channel_param1;
  int channel_param2;
  int channel_param3;
  int action;
  char *action_param;
  int planned_timestamp;
  int retry_timestamp;
  int retry_count;
} s_exec_t;

#define MIN_RETRY_TIME  5
#define MIN_CHECK_TIME  1

#define EXPIRE_TIME 300  // 5 min since planned_time
#define ZOMBIE_TIME 60   // 1 min since fetched_time

#define ONOFF_RETRY_LIMIT 2
#define ONOFF_RETRY_TIME 30

#define OPEN_RETRY_LIMIT 2
#define OPEN_RETRY_TIME 30

#define GATEOPENCLOSE_RETRY_LIMIT 4
#define GATEOPENCLOSE_RETRY_TIME 60

#define RS_RETRY_LIMIT 2
#define RS_RETRY_TIME 90

#define RGBW_RETRY_LIMIT 2
#define RGBW_RETRY_TIME 30

#define ACTION_OPEN 10
#define ACTION_CLOSE 20
#define ACTION_SHUT 30
#define ACTION_REVEAL 40
#define ACTION_REVEAL_PARTIALLY 50
#define ACTION_TURN_ON 60
#define ACTION_TURN_OFF 70
#define ACTION_SET_RGBW_PARAMETERS 80

#define ACTION_EXECUTION_RESULT_UNKNOWN 0
#define ACTION_EXECUTION_RESULT_SUCCESS 1
#define ACTION_EXECUTION_RESULT_DEVICE_UNREACHABLE 2
#define ACTION_EXECUTION_RESULT_NO_SENSOR 3
#define ACTION_EXECUTION_RESULT_EXPIRED 4
#define ACTION_EXECUTION_RESULT_ZOMBIE 5
#define ACTION_EXECUTION_RESULT_SERVER_UNREACHABLE 6
#define ACTION_EXECUTION_RESULT_FAILURE 7
#define ACTION_EXECUTION_RESULT_CANCELLED 8

#endif /*SUPLA_SCHEDULER_H_*/
