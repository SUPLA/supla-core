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

#include "queue.h"
#include "sthread.h"
#include "safearray.h"
#include "log.h"
#include "eh.h"
#include "schedulercfg.h"

char queue_loop_worker_thread_twt(void *worker_sthread) {
	sthread_twf(worker_sthread);
	return 1;
}

queue::queue() {

	max_workers = scfg_int(CFG_MAX_WORKERS);

	supla_log(LOG_DEBUG, "Maximum workers: %i", max_workers);

	workers_arr = safe_array_init();
	db = new database();

}

queue::~queue() {

	delete db;

	safe_array_clean(workers_arr, queue_loop_worker_thread_twt);
	safe_array_free(workers_arr);
}

void queue::load(void) {


}

void queue_loop(void *ssd, void *q_sthread) {

	TEventHandler *eh = eh_init();
	database::thread_init();
	queue *q = new queue();


	while(sthread_isterminated(q_sthread) == 0) {

		q->load();
		eh_wait(eh, 1000000);
	}

	delete q;
	database::thread_end();
	eh_free(eh);

}
