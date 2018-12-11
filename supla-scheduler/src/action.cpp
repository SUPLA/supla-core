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
#include <stdlib.h>
#include <string.h>
#include "log.h"

s_worker_action::s_worker_action(s_worker *worker) { this->worker = worker; }

s_worker_action::~s_worker_action() {}

bool s_worker_action::is_function_allowed(void) {
  int flist[FUNCTION_LIST_SIZE];
  memset((void *)flist, 0, FUNCTION_LIST_SIZE * sizeof(int));

  get_function_list(flist);

  for (int a = 0; a < FUNCTION_LIST_SIZE; a++)
    if (flist[a] == worker->get_channel_func()) return true;

  worker->get_db()->set_result(worker->get_id(),
                               ACTION_EXECUTION_RESULT_CANCELLED);
  return false;
}

bool s_worker_action::check_before_start(void) { return false; }

bool s_worker_action::no_sensor(void) { return false; }

bool s_worker_action::retry_when_fail(void) {
  return worker->retry_when_fail();
}

void s_worker_action::execute(void) {
  if (worker == NULL || worker->get_db() == NULL || !is_function_allowed())
    return;

  if (!worker->channel_group() && check_before_start() &&
      worker->get_retry_count() == 0 && check_result()) {
    worker->get_db()->set_result(worker->get_id(),
                                 ACTION_EXECUTION_RESULT_SUCCESS);
    return;
  }

  bool channel_group = worker->channel_group();

  if (worker->get_retry_count() % 2 == 0) {  // SET
    bool action_result = do_action();

    if (channel_group) {
      if (action_result) {
        worker->get_db()->set_result(worker->get_id(),
                                     ACTION_EXECUTED_WITHOUT_CONFIRMATION);
      } else {
        if (worker->ipcc_is_connected() == IPC_RESULT_SERVER_UNREACHABLE) {
          worker->get_db()->set_result(
              worker->get_id(), ACTION_EXECUTION_RESULT_SERVER_UNREACHABLE);
        } else {
          worker->get_db()->set_result(worker->get_id(),
                                       ACTION_EXECUTION_RESULT_FAILURE);
        }
      }

    } else {
      worker->get_db()->set_retry(worker->get_id(), waiting_time_to_check());
    }

    return;
  }

  // CHECK
  if (check_result()) {
    worker->get_db()->set_result(worker->get_id(),
                                 ACTION_EXECUTION_RESULT_SUCCESS);
  } else if (retry_when_fail() &&
             worker->get_retry_count() + 1 < (try_limit() * 2)) {
    worker->get_db()->set_retry(
        worker->get_id(), waiting_time_to_retry() - waiting_time_to_check());

  } else if (no_sensor()) {
    worker->get_db()->set_result(worker->get_id(),
                                 ACTION_EXECUTION_RESULT_NO_SENSOR);

  } else {
    switch (worker->ipcc_is_connected()) {
      case IPC_RESULT_CONNECTED:
        worker->get_db()->set_result(worker->get_id(),
                                     ACTION_EXECUTION_RESULT_FAILURE);
        break;
      case IPC_RESULT_DISCONNECTED:
        worker->get_db()->set_result(
            worker->get_id(), ACTION_EXECUTION_RESULT_DEVICE_UNREACHABLE);
        break;
      case IPC_RESULT_SERVER_UNREACHABLE:
        worker->get_db()->set_result(
            worker->get_id(), ACTION_EXECUTION_RESULT_SERVER_UNREACHABLE);
        break;
    }
  }
}

bool s_worker_action::parse_percentage(char *percent) {
  jsmn_parser p;
  jsmntok_t t[10];
  int a;
  int value = 0;

  if (worker->get_action_param() == NULL || percent == NULL) {
    return false;
  }

  jsmn_init(&p);
  int r = jsmn_parse(&p, worker->get_action_param(),
                     strnlen(worker->get_action_param(), 255), t,
                     sizeof(t) / sizeof(t[0]));

  if (r < 1 || t[0].type != JSMN_OBJECT) {
    return false;
  }

  for (a = 1; a < r - 1; a++) {
    if (jsoneq(worker->get_action_param(), &t[a], "percentage") == 0) {
      if (json_get_int(&t[a + 1], &value) && value >= 0 && value <= 100) {
        *percent = value;
        return true;
      }
    }
  }

  return false;
}

int s_worker_action::get_max_time(void) {
  return try_limit() * waiting_time_to_retry();
}

int s_worker_action::jsoneq(const char *json, jsmntok_t *tok, const char *s) {
  if (tok->type == JSMN_STRING &&
      (int)strnlen(s, 255) == tok->end - tok->start &&
      strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
    return 0;
  }
  return -1;
}

char s_worker_action::json_get_int(jsmntok_t *token, int *value) {
  char buffer[12];
  memset(buffer, 0, sizeof(buffer));

  if (value == NULL || token->type != JSMN_PRIMITIVE ||
      (unsigned int)(token->end - token->start) >= sizeof(buffer) ||
      token->end <= token->start)
    return 0;

  const char *action_param = worker->get_action_param();

  memcpy(buffer, &action_param[token->start], token->end - token->start);

  *value = atoi(buffer);

  return 1;
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
