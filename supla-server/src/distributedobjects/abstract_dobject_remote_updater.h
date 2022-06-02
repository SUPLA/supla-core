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

#ifndef SUPLA_ABSTRACT_DISTRIBUTED_OBJECT_REMOTE_UPDATER_H_
#define SUPLA_ABSTRACT_DISTRIBUTED_OBJECT_REMOTE_UPDATER_H_

#include "distributedobjects/dobject.h"

class supla_abstract_dobject_remote_updater {
 private:
  void *srpc;
  int protocol_version;

  bool transaction_started;
  bool transaction_should_end;
  bool any_updates;
  void begin_transaction(supla_dobject *object);

 protected:
  void *get_srpc(void);
  virtual void on_transaction_begin(supla_dobject *object) = 0;
  virtual void on_transaction_end(void *srpc, int protocol_version) = 0;

  virtual bool prepare_the_update(
      supla_dobject *object,
      supla_dobject_change_indicator **new_change_indicator,
      bool *transaction_should_end, int protocol_version) = 0;

 public:
  supla_abstract_dobject_remote_updater(void *srpc, int protocol_version);
  virtual ~supla_abstract_dobject_remote_updater();
  void update(supla_dobject *object);
  bool is_transaction_should_end(void);
  bool end_transaction(void);
};

#endif /* SUPLA_ABSTRACT_DISTRIBUTED_OBJECT_REMOTE_UPDATER_H_ */
