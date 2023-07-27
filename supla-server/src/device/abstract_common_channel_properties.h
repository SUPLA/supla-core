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

#ifndef ABSTRACT_COMMON_CHANNEL_PROPERTIES_H_
#define ABSTRACT_COMMON_CHANNEL_PROPERTIES_H_

#include <functional>
#include <vector>

#include "device/channel_relation.h"

enum e_relation_kind {
  relation_any = 0,
  relation_with_sub_channel = 1,
  relation_with_parent_channel = 2
};

class channel_json_config;
class supla_abstract_common_channel_properties {
 private:
  void add_relation(std::vector<supla_channel_relation> *relations,
                    int channel_id, int parent_id, short relation_type);

 protected:
  virtual int get_id(void) = 0;
  virtual int get_device_id(void) = 0;
  virtual int get_func(void) = 0;
  virtual int get_param1(void) = 0;
  virtual int get_param2(void) = 0;
  virtual int get_param3(void) = 0;
  virtual int get_param4(void) = 0;
  virtual channel_json_config *get_json_config(void) = 0;
  virtual unsigned char get_channel_number(void) = 0;

  virtual void for_each(
      std::function<void(supla_abstract_common_channel_properties *, bool *)>
          on_channel_properties) = 0;

 public:
  supla_abstract_common_channel_properties(void);
  virtual ~supla_abstract_common_channel_properties(void);

  std::vector<supla_channel_relation> get_channel_relations(
      e_relation_kind kind);

  void get_channel_relations(std::vector<supla_channel_relation> *relations,
                             e_relation_kind kind);
};

#endif /*ABSTRACT_COMMON_CHANNEL_PROPERTIES_H_*/
