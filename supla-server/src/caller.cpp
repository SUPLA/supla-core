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

#include "caller.h"

#include <string.h>

supla_caller::supla_caller(const supla_caller &caller) {
  this->parent = NULL;
  this->type = ctUnknown;
  this->id = 0;
  *this = caller;
}

supla_caller::supla_caller(void) {
  this->parent = NULL;
  this->type = ctUnknown;
  this->id = 0;
}

supla_caller::supla_caller(_callerType_e type) {
  this->parent = NULL;
  this->type = type;
  this->id = 0;
}

supla_caller::supla_caller(_callerType_e type, int id) {
  this->parent = NULL;
  this->type = type;
  this->id = id;
}

supla_caller::supla_caller(const supla_caller &parent, _callerType_e type) {
  this->parent = parent.clone();
  this->type = type;
  this->id = 0;
}

supla_caller::supla_caller(const supla_caller &parent, _callerType_e type,
                           int id) {
  this->parent = parent.clone();
  this->type = type;
  this->id = id;
}

supla_caller::~supla_caller() {
  if (parent) {
    delete parent;
    parent = NULL;
  }
}

const supla_caller *supla_caller::get_first(void) const {
  const supla_caller *caller = this;
  while (caller) {
    if (!caller->parent) {
      return caller;
    }
    caller = caller->parent;
  }

  return NULL;
}

int supla_caller::get_id(void) const { return get_first()->id; }

_callerType_e supla_caller::get_type() const { return get_first()->type; }

supla_caller *supla_caller::clone(void) const {
  supla_caller *result = new supla_caller();
  if (result) {
    result->parent = parent ? parent->clone() : NULL;
    result->type = type;
    result->id = id;
  }

  return result;
}

int supla_caller::convert_to_sender_id(void) const {
  if (type == ctClient) {
    return id;
  }
  return 0;
}

int supla_caller::find(_callerType_e type, int id) const {
  int count = 0;
  int n = 0;
  const supla_caller *caller = this;
  while (caller) {
    count++;
    if (caller->type == type && caller->id == id) {
      n = count;
    }

    caller = caller->parent;
  }

  return n ? count - n + 1 : 0;
}

int supla_caller::find(_callerType_e type) const { return find(type, 0); }

int supla_caller::stack_size(void) const {
  int result = 0;
  const supla_caller *caller = this;
  while (caller) {
    result++;
    caller = caller->parent;
  }

  return result;
}

supla_caller &supla_caller::operator=(const supla_caller &caller) {
  if (parent) {
    delete parent;
    parent = NULL;
  }

  parent = caller.parent ? caller.parent->clone() : NULL;
  type = caller.type;
  id = caller.id;

  return *this;
}

bool supla_caller::operator==(_callerType_e type) const {
  return get_type() == type;
}

bool supla_caller::operator!=(_callerType_e type) const {
  return get_type() != type;
}
