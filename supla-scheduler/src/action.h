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

#ifndef ACTION_H_
#define ACTION_H_

#include <list>
#include <string>
#include "jsmn.h"
#include "proto.h"
#include "worker.h"

#define FUNCTION_LIST_SIZE 10

class s_worker_action {
 private:
  bool check_function_allowed(void);

 protected:
  s_worker *worker;

  virtual void get_function_list(int list[FUNCTION_LIST_SIZE]) = 0;
  virtual int retry_limit(void) = 0;
  virtual void do_action(void) = 0;
  virtual bool check_result(void) = 0;

  virtual bool no_sensor(void);
  virtual bool retry_when_fail(void);

 public:
  virtual int waiting_time_to_retry(void) = 0;  // return seconds
  virtual int waiting_time_to_check(void) = 0;  // return seconds
  explicit s_worker_action(s_worker *worker);
  virtual ~s_worker_action();
  void execute(void);

  bool parse_percentage(char *percent);
  int jsoneq(const char *json, jsmntok_t *tok, const char *s);
  char json_get_int(jsmntok_t *token, int *value);
};

class AbstractActionFactory {
 private:
  int action_type;
  std::string classname;

 public:
  explicit AbstractActionFactory(int action_type, std::string classname);
  std::string getActionClassName(void);
  int getActionType(void);
  virtual s_worker_action *create(s_worker *worker) = 0;
  virtual ~AbstractActionFactory(void);

  static AbstractActionFactory *factoryByActionType(int action_type);
  static s_worker_action *createByActionType(int action_type, s_worker *worker);
  static std::list<AbstractActionFactory *> factories;
};

#define REGISTER_ACTION(actionclass, actiontype)                    \
  class actionclass##Factory : public AbstractActionFactory {       \
   public:                                                          \
    actionclass##Factory();                                         \
    s_worker_action *create(s_worker *worker);                      \
  };                                                                \
  actionclass##Factory::actionclass##Factory()                      \
      : AbstractActionFactory(actiontype, #actionclass) {}          \
  s_worker_action *actionclass##Factory::create(s_worker *worker) { \
    return new actionclass(worker);                                 \
  }                                                                 \
  static actionclass##Factory global_##actionclass##Factory;

#endif /*ACTION_H_*/
