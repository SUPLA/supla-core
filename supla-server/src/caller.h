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

#ifndef CALLER_H_
#define CALLER_H_

#include <string>

enum _callerType_e {
  ctUnknown,
  ctDevice,
  ctClient,
  ctIPC,
  ctMQTT,
  ctAmazonAlexa,
  ctGoogleHome,
  ctActionTrigger,
  ctScene,
  ctCyclicTask,
  ctChannel
};

class supla_caller {
 private:
  supla_caller *parent;
  _callerType_e type;
  int id;
  std::string name;
  const supla_caller *get_first(void) const;

 public:
  supla_caller(const supla_caller &caller);  // NOLINT
  supla_caller(void);
  explicit supla_caller(_callerType_e type);
  supla_caller(_callerType_e type, int id);
  supla_caller(_callerType_e type, int id, const std::string &name);
  supla_caller(const supla_caller &parent, _callerType_e type);
  supla_caller(const supla_caller &parent, _callerType_e type, int id);
  supla_caller(const supla_caller &parent, _callerType_e type, int id,
               const std::string &name);
  virtual ~supla_caller();
  int get_id(void) const;
  _callerType_e get_type() const;
  const std::string &get_name(void) const;
  supla_caller *clone(void) const;
  int convert_to_sender_id(void) const;
  int find(_callerType_e type, int id) const;
  int find(_callerType_e type) const;
  int stack_size(void) const;
#ifdef __DEBUG
  void print_stack(void) const;
#endif /*__DEBUG*/
  supla_caller &operator=(const supla_caller &caller);
  bool operator==(_callerType_e type) const;
  bool operator!=(_callerType_e type) const;
  bool operator==(const supla_caller &caller) const;
  bool operator!=(const supla_caller &caller) const;
};

#endif /* CALLER_H_ */
