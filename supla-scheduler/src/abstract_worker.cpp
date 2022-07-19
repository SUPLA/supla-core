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

#include "abstract_worker.h"

#include "action.h"
#include "log.h"
#include "sthread.h"

s_abstract_worker::s_abstract_worker(queue *q) {
  db = new database();
  ipcc = new ipc_client();
  this->q = q;
}

s_abstract_worker::~s_abstract_worker(void) {
  delete db;
  delete ipcc;
}

database *s_abstract_worker::get_db(void) { return db; }

ipc_client *s_abstract_worker::get_ipcc(void) { return ipcc; }

queue *s_abstract_worker::get_queue(void) { return q; }

const s_exec_params_t *s_abstract_worker::get_params(void) { return &s_exec_params; }

void s_abstract_worker::execute(void *sthread) {
  if (!db->connect()) return;

  s_exec_params = q->get_job();

  while (s_exec_params.id && !sthread_isterminated(sthread)) {
    if (db->set_fetched(s_exec_params.id)) q->mark_fetched();

    s_worker_action *action =
        AbstractActionFactory::createByActionType(s_exec_params.action, this);

    if (action) {
      action->execute();
      delete action;
    } else {
      db->set_result(s_exec_params.id, ACTION_EXECUTION_RESULT_CANCELLED);
      supla_log(LOG_ERR, "Action %i is not supported!", s_exec_params.action);
    }

    if (s_exec_params.action_param != NULL) free(s_exec_params.action_param);

    s_exec_params = q->get_job();
  }

  q->raise_loop_event();
}
