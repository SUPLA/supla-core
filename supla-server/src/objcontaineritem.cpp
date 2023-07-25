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

#include "objcontaineritem.h"

#include "lck.h"
#include "log.h"

supla_objcontainer_item::supla_objcontainer_item(supla_objcontainer *Container,
                                                 int Id) {
  this->Container = Container;
  this->Id = Id;
  this->RemoteUpdateMark = 0;
  this->lck = lck_init();
}

supla_objcontainer_item::~supla_objcontainer_item(void) { lck_free(lck); }

supla_objcontainer *supla_objcontainer_item::getContainer(void) {
  return Container;
}

int supla_objcontainer_item::get_id() { return Id; }

int supla_objcontainer_item::get_extra_id() { return -1; }

void supla_objcontainer_item::mark_for_remote_update(int mark) {
  if (remote_update_is_possible()) {
    lock();
    RemoteUpdateMark |= mark;
    unlock();
  }
}

void supla_objcontainer_item::unmark_for_remote_update(int unmark) {
  lock();
  RemoteUpdateMark ^= RemoteUpdateMark & unmark;
  unlock();
}

int supla_objcontainer_item::marked_for_remote_update(void) {
  lock();
  int result = RemoteUpdateMark;
  unlock();
  return result;
}

void supla_objcontainer_item::lock(void) { lck_lock(lck); }

void supla_objcontainer_item::unlock(void) { lck_unlock(lck); }
