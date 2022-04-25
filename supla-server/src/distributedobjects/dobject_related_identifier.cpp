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

#include "distributedobjects/dobject_related_identifier.h"

#include <cstddef>

supla_dobject_related_identifier::supla_dobject_related_identifier(int id) {
  this->id = id;
  this->rui = NULL;
}

supla_dobject_related_identifier::~supla_dobject_related_identifier() {
  if (rui) {
    delete rui;
  }
}

int supla_dobject_related_identifier::get_id(void) { return id; }

void supla_dobject_related_identifier::set_remote_update_indicator(
    supla_dobject_remote_update_indicator* rui) {
  supla_dobject_remote_update_indicator* old = this->rui;
  this->rui = rui;
  if (rui && old) {
    rui->apply_previous(old);
  }

  if (old) {
    delete old;
  }
}

const supla_dobject_remote_update_indicator*
supla_dobject_related_identifier::get_remote_update_indicator(void) const {
  return rui;
}
