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

#include "action.h"
#include <assert.h>
#include <string.h>
#include "log.h"

s_worker_action::s_worker_action(s_worker *worker) { this->worker = worker; }

s_worker_action::~s_worker_action() {}

bool s_worker_action::check_function_allowed(void) {
  int flist[FUNCTION_LIST_SIZE];
  memset((void *)flist, 0, FUNCTION_LIST_SIZE * sizeof(int));

  get_function_list(flist);

  for (int a = 0; a < FUNCTION_LIST_SIZE; a++)
    if (flist[a] == worker->get_channel_func()) return true;

  worker->get_db()->set_result(worker->get_id(),
                               ACTION_EXECUTION_RESULT_CANCELLED);
  return false;
}

bool s_worker_action::no_sensor(void) { return false; }

bool s_worker_action::retry_when_fail(void) {
  return worker->retry_when_fail();
}

void s_worker_action::execute(void) {
  if (worker == NULL || worker->get_db() == NULL || !check_function_allowed())
    return;

  supla_log(LOG_DEBUG, "EXECUTE %i", worker->get_retry_count());

  if (worker->get_retry_count() % 2 == 0) {  // SET
    supla_log(LOG_DEBUG, "DO ACTION");
    do_action();
    worker->get_db()->set_retry(worker->get_id(), waiting_time_to_check());
    return;
  }

  // CHECK
  supla_log(LOG_DEBUG, "CHECK RESULT");
  if (check_result()) {
    worker->get_db()->set_result(worker->get_id(),
                                 ACTION_EXECUTION_RESULT_SUCCESS);
  } else if (retry_when_fail() &&
             worker->get_retry_count()+1 < (retry_limit() * 2)) {
    supla_log(LOG_DEBUG, "RETRY WHEN FAIL");
    worker->get_db()->set_retry(
        worker->get_id(), waiting_time_to_retry() - waiting_time_to_check());

  } else if (no_sensor()) {
    supla_log(LOG_DEBUG, "NO SENSOR");
    worker->get_db()->set_result(worker->get_id(),
                                 ACTION_EXECUTION_RESULT_NO_SENSOR);

  } else {
    switch (worker->ipcc_is_connected()) {
      case IPC_RESULT_CONNECTED:
        worker->get_db()->set_result(worker->get_id(),
                                     ACTION_EXECUTION_RESULT_FAILURE);
        supla_log(LOG_DEBUG, "RESULT_FAILURE");
        break;
      case IPC_RESULT_DISCONNECTED:
        worker->get_db()->set_result(
            worker->get_id(), ACTION_EXECUTION_RESULT_DEVICE_UNREACHABLE);
        supla_log(LOG_DEBUG, "DEVICE_UNREACHABLE");
        break;
      case IPC_RESULT_SERVER_UNREACHABLE:
        worker->get_db()->set_result(
            worker->get_id(), ACTION_EXECUTION_RESULT_SERVER_UNREACHABLE);
        supla_log(LOG_DEBUG, "SERVER_UNREACHABLE");
        break;
    }
  }
}

//-----------------------------------------------------------------
//-----------------------------------------------------------------
//-----------------------------------------------------------------

std::list<AbstractActionFactory *> AbstractActionFactory::factories;

AbstractActionFactory::~AbstractActionFactory(void) {}

AbstractActionFactory::AbstractActionFactory(int action_type,
                                             std::string classname) {
  assert(AbstractActionFactory::factoryByActionType(action_type) == NULL);

  this->action_type = action_type;
  this->classname = classname;
  factories.push_back(this);
}

int AbstractActionFactory::getActionType(void) { return action_type; }

std::string AbstractActionFactory::getActionClassName(void) {
  return classname;
}

AbstractActionFactory *AbstractActionFactory::factoryByActionType(
    int action_type) {
  for (std::list<AbstractActionFactory *>::iterator it =
           AbstractActionFactory::factories.begin();
       it != AbstractActionFactory::factories.end(); it++) {
    if ((*it)->getActionType() == action_type) return *it;
  }

  return NULL;
}

s_worker_action *AbstractActionFactory::createByActionType(int action_type,
                                                           s_worker *worker) {
  AbstractActionFactory *factory = NULL;

  if (NULL !=
      (factory = AbstractActionFactory::factoryByActionType(action_type))) {
    return factory->create(worker);
  }

  return NULL;
}
