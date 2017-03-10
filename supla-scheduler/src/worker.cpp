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

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "worker.h"
#include "sthread.h"
#include "log.h"
#include "tools.h"
#include "ipcclient.h"


s_worker::s_worker(s_exec_t *s_exec) {

	memcpy(&this->s_exec, s_exec, sizeof(s_exec_t));
	db = new database();
	ipcc = new ipc_client();
}


s_worker::~s_worker(void) {
	delete db;
	delete ipcc;
}

void s_worker::set_result(bool success, int retry_limit, int retry_time, bool no_sensor) {

	if ( success ) {

		//supla_log(LOG_DEBUG, "SUCCESS");
		db->set_result(s_exec.id, ACTION_EXECUTION_RESULT_SUCCESS);

	} else if ( s_exec.retry_count < retry_limit ) {

		//supla_log(LOG_DEBUG, "RETRY %i", s_exec.retry_count);
		db->set_retry(s_exec.id, retry_time);

	} else if ( no_sensor ) {

		db->set_result(s_exec.id, ACTION_EXECUTION_RESULT_NO_SENSOR);

	} else {

		switch(ipcc->is_connected(s_exec.user_id, s_exec.device_id)) {
		case IPC_RESULT_CONNECTED:
			db->set_result(s_exec.id, ACTION_EXECUTION_RESULT_FAILURE);
			//supla_log(LOG_DEBUG, "RESULT_FAILURE");
			break;
		case IPC_RESULT_DISCONNECTED:
			db->set_result(s_exec.id, ACTION_EXECUTION_RESULT_DEVICE_UNREACHABLE);
			//supla_log(LOG_DEBUG, "DEVICE_UNREACHABLE");
			break;
		case IPC_RESULT_SERVER_UNREACHABLE:
			db->set_result(s_exec.id, ACTION_EXECUTION_RESULT_SERVER_UNREACHABLE);
			//supla_log(LOG_DEBUG, "SERVER_UNREACHABLE");
			break;
		}

	}
}

void s_worker::action_turn_on_off(char on) {

	supla_log(LOG_DEBUG, "ACTION ON/OFF channel:%i try:%i", s_exec.channel_id, s_exec.retry_count+1);

	bool success = ipcc->set_char_value(s_exec.user_id, s_exec.device_id, s_exec.channel_id, on == 1 ? 1 : 0);
	set_result(success, ONOFF_RETRY_LIMIT, ONOFF_RETRY_TIME, false);
}

void s_worker::execute(void *sthread) {

	if ( !db->connect()
		 || !db->set_fetched(s_exec.id) )
		return;

	time_t now_utc = st_get_utc_time();
	int delay = now_utc-s_exec.planned_timestamp;

	if ( delay < 0 )
		sleep(delay*-1);

	if ( sthread_isterminated(sthread) )
		db->set_unfetched(s_exec.id);


	switch(s_exec.action) {
	case ACTION_OPEN:
		break;
	case ACTION_CLOSE:
		break;
	case ACTION_SHUT:
		break;
	case ACTION_REVEAL:
		break;
	case ACTION_TURN_ON:
		action_turn_on_off(1);
		break;
	case ACTION_TURN_OFF:
		action_turn_on_off(0);
		break;
	case ACTION_DIM:
		break;
	case ACTION_SET_RGB_COLOR:
		break;
	case ACTION_SET_RANDOM_RGB_COLOR:
		break;
	}


}
