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

#include <my_global.h>
#include <mysql.h>
#include <string.h>

#include "database.h"
#include "schedulercfg.h"
#include "log.h"
#include "tools.h"

char *database::cfg_get_host(void) {

	return scfg_string(CFG_MYSQL_HOST);
}

char *database::cfg_get_user(void) {

	return scfg_string(CFG_MYSQL_USER);
}
char *database::cfg_get_password(void) {

	return scfg_string(CFG_MYSQL_PASSWORD);
}
char *database::cfg_get_database(void) {

	return scfg_string(CFG_MYSQL_DB);
}
int database::cfg_get_port(void) {

	return scfg_int(CFG_MYSQL_PORT);
}

void database::set_overdue_result(int overdue_time) {

	MYSQL_STMT *stmt;
	MYSQL_BIND pbind[2];
	memset(pbind, 0, sizeof(pbind));

	int result = ACTION_EXECUTION_RESULT_OVERDUE;

	pbind[0].buffer_type= MYSQL_TYPE_LONG;
	pbind[0].buffer= (char *)&result;

	pbind[1].buffer_type= MYSQL_TYPE_LONG;
	pbind[1].buffer= (char *)&overdue_time;

	if ( stmt_execute((void**)&stmt, "UPDATE `supla_scheduled_executions` SET `result_timestamp`= UTC_TIMESTAMP(), `result` = ? WHERE fetched_timestamp IS NULL AND planned_timestamp + INTERVAL ? SECOND <= UTC_TIMESTAMP()", pbind, 2, true) ) {
		mysql_stmt_close(stmt);
	}

}

void database::set_zombie_result(int zombie_time) {

	MYSQL_STMT *stmt;
	MYSQL_BIND pbind[2];
	memset(pbind, 0, sizeof(pbind));

	int result = ACTION_EXECUTION_RESULT_ZOMBIE;

	pbind[0].buffer_type= MYSQL_TYPE_LONG;
	pbind[0].buffer= (char *)&result;

	pbind[1].buffer_type= MYSQL_TYPE_LONG;
	pbind[1].buffer= (char *)&zombie_time;

	if ( stmt_execute((void**)&stmt, "UPDATE `supla_scheduled_executions` SET `result_timestamp`= UTC_TIMESTAMP(), `result` = ? WHERE result IS NULL AND fetched_timestamp IS NOT NULL AND fetched_timestamp + INTERVAL ? SECOND <= UTC_TIMESTAMP()", pbind, 2, true) ) {
		mysql_stmt_close(stmt);
	}

}

int database::get_s_executions(s_exec_t **s_exec, int limit) {

	MYSQL_STMT *stmt;

	MYSQL_BIND pbind[1];
	memset(pbind, 0, sizeof(pbind));

	int result = 0;
	*s_exec = NULL;

	pbind[0].buffer_type= MYSQL_TYPE_LONG;
	pbind[0].buffer= (char *)&limit;

	if ( stmt_execute((void**)&stmt, "SELECT e.`id`, e.`schedule_id`, s.`user_id`, s.`channel_id`, s.`action`, s.`action_param`, UNIX_TIMESTAMP(e.`planned_timestamp`), UNIX_TIMESTAMP(e.`retry_timestamp`), retry_count FROM `supla_scheduled_executions` AS e, `supla_schedule` AS s WHERE e.`schedule_id` = s.`id` AND `fetched_timestamp` IS NULL AND ( (retry_timestamp IS NULL AND planned_timestamp <= UTC_TIMESTAMP()) OR (retry_timestamp IS NOT NULL AND retry_timestamp <= UTC_TIMESTAMP())) LIMIT ?", pbind, 1, true) ) {

		my_bool is_null[3];

		MYSQL_BIND rbind[9];
		memset(rbind, 0, sizeof(rbind));

		int id, schedule_id, user_id, channel_id, action, planned_timestamp, retry_timestamp, retry_count;
		unsigned long length;
		char action_param[256];


		rbind[0].buffer_type= MYSQL_TYPE_LONG;
		rbind[0].buffer= (char *)&id;

		rbind[1].buffer_type= MYSQL_TYPE_LONG;
		rbind[1].buffer= (char *)&schedule_id;

		rbind[2].buffer_type= MYSQL_TYPE_LONG;
		rbind[2].buffer= (char *)&user_id;

		rbind[3].buffer_type= MYSQL_TYPE_LONG;
		rbind[3].buffer= (char *)&channel_id;

		rbind[4].buffer_type= MYSQL_TYPE_LONG;
		rbind[4].buffer= (char *)&action;

		rbind[5].buffer_type= MYSQL_TYPE_STRING;
		rbind[5].buffer= action_param;
		rbind[5].is_null= &is_null[0];
		rbind[5].buffer_length = 256;
		rbind[5].length = &length;

		rbind[6].buffer_type= MYSQL_TYPE_LONG;
		rbind[6].buffer= &planned_timestamp;

		rbind[7].buffer_type= MYSQL_TYPE_LONG;
		rbind[7].buffer= &retry_timestamp;
		rbind[7].is_null= &is_null[1];

		rbind[8].buffer_type= MYSQL_TYPE_LONG;
		rbind[8].buffer= &retry_count;
		rbind[8].is_null= &is_null[2];

		if ( mysql_stmt_bind_result(stmt, rbind) ) {
			supla_log(LOG_ERR, "MySQL - stmt bind error - %s", mysql_stmt_error(stmt));
		} else {

			mysql_stmt_store_result(stmt);

			if ( mysql_stmt_num_rows(stmt) > 0 ) {

				int s_exec_size = sizeof(s_exec_t) * mysql_stmt_num_rows(stmt);
				*s_exec = (s_exec_t*)malloc(s_exec_size);
				memset(*s_exec, 0, s_exec_size);

				while (!mysql_stmt_fetch(stmt)) {

					(*s_exec)[result].id = id;
					(*s_exec)[result].schedule_id = schedule_id;
					(*s_exec)[result].user_id = user_id;
					(*s_exec)[result].channel_id = channel_id;
					(*s_exec)[result].action = action;
					(*s_exec)[result].planned_timestamp = planned_timestamp;

					if ( is_null[0] == false ) {
						action_param[255] = 0;
						(*s_exec)[result].action_param = atoi(action_param);
					}

					(*s_exec)[result].retry_timestamp = is_null[1] ? 0 : retry_timestamp;
					(*s_exec)[result].retry_count = is_null[2] ? 0 : retry_count;

					result++;

				}

			}

		}


		mysql_stmt_close(stmt);
	}

	return result;
}

bool database::set_fetched(int id) {

	bool result = false;
	MYSQL_STMT *stmt;
	MYSQL_BIND pbind[1];
	memset(pbind, 0, sizeof(pbind));

	pbind[0].buffer_type= MYSQL_TYPE_LONG;
	pbind[0].buffer= (char *)&id;


	if ( stmt_execute((void**)&stmt, "UPDATE `supla_scheduled_executions` SET `fetched_timestamp`= UTC_TIMESTAMP() WHERE `id` = ? AND `fetched_timestamp` IS NULL", pbind, 1, true) ) {

		result = mysql_stmt_affected_rows(stmt) == 1;
		mysql_stmt_close(stmt);
	}

	return result;
}

void database::set_unfetched(int id) {

	MYSQL_STMT *stmt;
	MYSQL_BIND pbind[1];
	memset(pbind, 0, sizeof(pbind));

	pbind[0].buffer_type= MYSQL_TYPE_LONG;
	pbind[0].buffer= (char *)&id;

	if ( stmt_execute((void**)&stmt, "UPDATE `supla_scheduled_executions` SET `fetched_timestamp`= NULL WHERE `id` = ?", pbind, 1, true) ) {
		mysql_stmt_close(stmt);
	}

}
