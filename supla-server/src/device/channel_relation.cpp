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

#include "device/channel_relation.h"

#include "distributedobjects/dobject_change_indicator.h"

supla_channel_relation::supla_channel_relation(int channel_id, int parent_id,
                                               short relation_type)
    : supla_dobject(channel_id) {
  this->parent_id = parent_id;
  this->relation_type = relation_type;
}

supla_channel_relation::supla_channel_relation(supla_channel_relation *relation,
                                               bool changed)
    : supla_dobject(relation->get_id()) {
  this->parent_id = relation->get_parent_id();
  this->relation_type = relation->get_relation_type();

  this->set_change_indicator(new supla_dobject_change_indicator(changed));
}

supla_channel_relation::~supla_channel_relation() {}

int supla_channel_relation::get_parent_id(void) { return parent_id; }

void supla_channel_relation::set_parent_id(int parent_id) {
  this->parent_id = parent_id;
}

short supla_channel_relation::get_relation_type(void) { return relation_type; }

void supla_channel_relation::set_relation_type(short relation_type) {
  this->relation_type = relation_type;
}

void supla_channel_relation::convert(TSC_SuplaChannelRelation *dest) {
  *dest = {};

  dest->Id = get_id();
  dest->ParentId = get_parent_id();
  dest->Type = get_relation_type();
}
