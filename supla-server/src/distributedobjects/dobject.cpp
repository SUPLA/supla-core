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

#include "distributedobjects/dobject.h"

#include <cstddef>

supla_dobject::supla_dobject(int id) {
  this->id = id;
  this->change_indicator = NULL;
}

supla_dobject::~supla_dobject() {
  if (change_indicator) {
    delete change_indicator;
    change_indicator = NULL;
  }
}

int supla_dobject::get_id(void) { return id; }

void supla_dobject::set_change_indicator(
    supla_dobject_change_indicator *change_indicator) {
  if (this->change_indicator == change_indicator) {
    return;
  }

  if (this->change_indicator) {
    delete this->change_indicator;
  }

  this->change_indicator = change_indicator;
}

const supla_dobject_change_indicator *supla_dobject::get_change_indicator(
    void) {
  return change_indicator;
}
