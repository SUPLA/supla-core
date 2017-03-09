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

}


s_worker::~s_worker(void) {
	delete db;
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


	ipc_client *ipcc = new ipc_client();

	supla_log(LOG_DEBUG, "IPC AUTH: %i", ipcc->auth());

	delete ipcc;
	supla_log(LOG_DEBUG, "WORK!, %i, %i", now_utc-s_exec.planned_timestamp, now_utc-s_exec.retry_timestamp);

}
