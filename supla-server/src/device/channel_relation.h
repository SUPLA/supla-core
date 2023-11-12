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

#ifndef SUPLA_CHANNEL_RELATION_H_
#define SUPLA_CHANNEL_RELATION_H_

#include "distributedobjects/dobject.h"
#include "proto.h"

class supla_channel_relation : public supla_dobject {
 private:
  int parent_id;
  short relation_type;

 public:
  supla_channel_relation(int channel_id, int parent_id, short relation_type);
  supla_channel_relation(supla_channel_relation *relation, bool changed);
  virtual ~supla_channel_relation();

  int get_parent_id(void);
  void set_parent_id(int parent_id);
  short get_relation_type(void);
  void set_relation_type(short relation_type);

  void convert(TSC_SuplaChannelRelation *dest);
};

#endif /* SUPLA_CHANNEL_RELATION_H_ */
