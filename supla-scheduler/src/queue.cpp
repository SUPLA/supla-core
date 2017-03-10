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

#include "queue.h"
#include "sthread.h"
#include "safearray.h"
#include "log.h"
#include "eh.h"
#include "schedulercfg.h"
#include "worker.h"
#include "ipcclient.h"

char queue_loop_worker_thread_twt(void *worker_sthread) {
	sthread_twf(worker_sthread);
	return 1;
}

char queue_loop_worker_thread_cnd(void *worker_sthread) {
   if ( sthread_isfinished(worker_sthread) == 1 ) {

	   sthread_free(worker_sthread);
	   return 1;
   }

   return 0;
}

void queue_loop_worker_execute(void *worker, void *sthread) {

	database::thread_init();
	((s_worker *)worker)->execute(sthread);

}

void queue_loop_worker_finish(void *worker, void *sthread) {

	delete (s_worker *)worker;
	database::thread_end();
}

queue::queue() {

	max_workers = scfg_int(CFG_MAX_WORKERS);
	s_exec = NULL;

	workers_thread_arr = safe_array_init();
	db = new database();

}

queue::~queue() {

	delete db;

	safe_array_clean(workers_thread_arr, queue_loop_worker_thread_twt);
	safe_array_free(workers_thread_arr);

	if ( s_exec != NULL ) {
		free(s_exec);
		s_exec = NULL;
	}
}

void queue::set_overdue_result(void) {

	database *_db = new database();
	if ( _db->connect() ) {

		_db->set_overdue_result(OVERDUE_TIME);

	};
	delete _db;

}

void queue::set_zombie_result(void) {

	database *_db = new database();
	if ( _db->connect() ) {

		_db->set_zombie_result(ZOMBIE_TIME);

	};
	delete _db;

}

void queue::load(void) {

	safe_array_clean(workers_thread_arr, queue_loop_worker_thread_cnd);

	int _max_workers = max_workers - safe_array_count(workers_thread_arr);

	if ( _max_workers <= 0 )
		return;

	if ( s_exec != NULL ) {
		free(s_exec);
		s_exec = NULL;
	}

	if ( !db->connect() )
		return;


	int count = db->get_s_executions(&s_exec, _max_workers);

	//supla_log(LOG_DEBUG, "Count: %i/%i", count, _max_workers);

	for(int a=0;a<count;a++) {

		Tsthread_params stp;

		stp.execute = queue_loop_worker_execute;
		stp.finish = queue_loop_worker_finish;
		stp.user_data = new s_worker(&s_exec[a]);
		stp.free_on_finish = 0;
		stp.initialize = NULL;

		safe_array_add(workers_thread_arr, sthread_run(&stp));

	}

	db->disconnect();

}

void queue_loop(void *ssd, void *q_sthread) {

	TEventHandler *eh = eh_init();
	database::thread_init();
	queue *q = new queue();

	//q->set_zombie_result();
	//q->set_overdue_result();

	while(sthread_isterminated(q_sthread) == 0) {

		q->load();
		eh_wait(eh, 1000000);
	}

	delete q;
	database::thread_end();
	eh_free(eh);

}
