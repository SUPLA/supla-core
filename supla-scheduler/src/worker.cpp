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
#include "queue.h"


s_worker::s_worker(queue *q) {

	db = new database();
	ipcc = new ipc_client();
	this->q = q;
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

		//supla_log(LOG_DEBUG, "NO SENSOR");
		db->set_result(s_exec.id, ACTION_EXECUTION_RESULT_NO_SENSOR);

	} else {

		switch(ipcc->is_connected(s_exec.user_id, s_exec.iodevice_id)) {
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

bool s_worker::check_function_allowed(int *func, int func_count) {

	int a;
	for(a=0;a<func_count;a++)
		if ( func[a] == s_exec.channel_func ) return true;

	db->set_result(s_exec.id, ACTION_EXECUTION_RESULT_CANCELLED);
	return false;
}

char s_worker::opening_sensor_value() {

	char value = -1;

	supla_channel sensor_channel;
	memset(&sensor_channel, 0, sizeof(supla_channel));

	sensor_channel.id = s_exec.channel_param2;

	if ( sensor_channel.id != 0
		 && db->get_channel(&sensor_channel)
		 && sensor_channel.param1 == s_exec.channel_id
		 && ( sensor_channel.type == SUPLA_CHANNELTYPE_SENSORNO || sensor_channel.type == SUPLA_CHANNELTYPE_SENSORNC )
		 && ipcc->is_connected(s_exec.user_id, sensor_channel.iodevice_id) == IPC_RESULT_CONNECTED ) {

 		if ( !ipcc->get_char_value(s_exec.user_id, sensor_channel.iodevice_id, sensor_channel.id, &value) ) {
			value = -1;
		} else {

			value = !!value;

			if ( sensor_channel.type == SUPLA_CHANNELTYPE_SENSORNC )
				value = value == 1 ? 0 : 1;

		}

	}

	return value;
}

void s_worker::action_turn_on_off(char on) {

	int func[] = { SUPLA_CHANNELFNC_LIGHTSWITCH,
			       SUPLA_CHANNELFNC_POWERSWITCH };

	if ( !check_function_allowed(func, sizeof(func)/sizeof(int)) )
		return;

	bool success = ipcc->set_char_value(s_exec.user_id, s_exec.iodevice_id, s_exec.channel_id, on == 1 ? 1 : 0);
	set_result(success, ONOFF_RETRY_LIMIT, ONOFF_RETRY_TIME, false);
}

void s_worker::action_gate_open_close(char _close) {

	int func[] = { SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR,
			       SUPLA_CHANNELFNC_CONTROLLINGTHEGATE };

	if ( !check_function_allowed(func, sizeof(func)/sizeof(int)) )
		return;

	//supla_log(LOG_DEBUG, "ACTION OPEN/CLOSE channel:%i try:%i", s_exec.channel_id, s_exec.retry_count+1);

	bool success = false;
	char sensor_value = opening_sensor_value();

	if ( sensor_value != -1 ) {

		if ( sensor_value == _close ) {
			success = true;
		} else if ( s_exec.retry_count < GATEOPENCLOSE_RETRY_LIMIT ) { // last one only for sensor check
			ipcc->set_char_value(s_exec.user_id, s_exec.iodevice_id, s_exec.channel_id, 1);
		}
	}

	set_result(success, GATEOPENCLOSE_RETRY_LIMIT, GATEOPENCLOSE_RETRY_TIME, sensor_value == -1);


}

void s_worker::action_shut_reveal(char shut) {

	int func[] = { SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER };

	if ( !check_function_allowed(func, sizeof(func)/sizeof(int)) )
		return;

	bool success = false;
	char sensor_value = opening_sensor_value();

	if ( sensor_value != shut
		 || sensor_value == -1 ) {

		bool _s = ipcc->set_char_value(s_exec.user_id, s_exec.iodevice_id, s_exec.channel_id, shut == 1 ? 3 : 4);

		if ( sensor_value == -1 )
			success = _s;

	} else if ( sensor_value == shut ) {
		success = true;
	}

	set_result(success, RS_RETRY_LIMIT, RS_RETRY_TIME, false);

}

void s_worker::execute(void *sthread) {

	if ( !db->connect() )
		return;

	s_exec = q->get_job();

	while( s_exec.id
		   && !sthread_isterminated(sthread)) {

		if ( db->set_fetched(s_exec.id) )
			q->mark_fetched();

		switch(s_exec.action) {
		case ACTION_CLOSE:
			action_gate_open_close(1);
			break;
		case ACTION_OPEN:
			action_gate_open_close(0);
			break;
		case ACTION_SHUT:
			action_shut_reveal(1);
			break;
		case ACTION_REVEAL:
			action_shut_reveal(0);
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

		s_exec = q->get_job();

	}

	q->raise_loop_event();


}
