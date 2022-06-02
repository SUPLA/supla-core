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

#include <distributedobjects/abstract_dobject_remote_updater.h>

supla_abstract_dobject_remote_updater::supla_abstract_dobject_remote_updater(
    void *srpc, int protocol_version) {
  this->srpc = srpc;
  this->protocol_version = protocol_version;
  this->transaction_started = false;
  this->transaction_should_end = false;
  this->any_updates = false;
}

supla_abstract_dobject_remote_updater::
    ~supla_abstract_dobject_remote_updater() {}

void *supla_abstract_dobject_remote_updater::get_srpc(void) { return srpc; }

void supla_abstract_dobject_remote_updater::begin_transaction(
    supla_dobject *object) {
  if (!transaction_started) {
    on_transaction_begin(object);

    transaction_started = true;
    transaction_should_end = false;
    any_updates = false;
  }
}

void supla_abstract_dobject_remote_updater::update(supla_dobject *object) {
  const supla_dobject_change_indicator *change_indicator =
      object->get_change_indicator();

  if (change_indicator && change_indicator->is_changed()) {
    if (!transaction_started) {
      begin_transaction(object);
    }

    supla_dobject_change_indicator *new_change_indicator = NULL;
    if (prepare_the_update(object, &new_change_indicator,
                           &transaction_should_end, protocol_version)) {
      object->set_change_indicator(new_change_indicator);
      any_updates = true;

    } else if (new_change_indicator) {
      delete new_change_indicator;
      new_change_indicator = NULL;
    }
  }
}

bool supla_abstract_dobject_remote_updater::is_transaction_should_end(void) {
  if (transaction_started) {
    return transaction_should_end;
  }

  return false;
}

bool supla_abstract_dobject_remote_updater::end_transaction(void) {
  if (transaction_started) {
    on_transaction_end(srpc, protocol_version);

    transaction_started = false;
    return any_updates;
  }

  return false;
}
