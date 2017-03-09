/*
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
	int schedule_id;
	int user_id;
	int channel_id;
	int action;
	int action_param;
	int planned_timestamp;
	int retry_timestamp;
	int retry_count;

}s_exec_t;

#define OVERDUE_TIME   300 // 5 min since planned_time
#define ZOMBIE_TIME    60  // 1 min since fetched_time

#define ACTION_EXECUTION_RESULT_UNKNOWN              0
#define ACTION_EXECUTION_RESULT_SUCCESS              1
#define ACTION_EXECUTION_RESULT_DEVICE_UNREACHABLE   2
#define ACTION_EXECUTION_RESULT_NO_SENSOR            3
#define ACTION_EXECUTION_RESULT_OVERDUE              4
#define ACTION_EXECUTION_RESULT_ZOMBIE               5

#endif /*SUPLA_SCHEDULER_H_*/
