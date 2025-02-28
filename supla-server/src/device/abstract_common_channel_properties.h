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

class supla_json_config;
class supla_abstract_common_channel_properties {
 private:
  void add_relation(std::vector<supla_channel_relation> *relations,
                    int channel_id, int parent_id, short relation_type);
  template <typename config_class_T, typename raw_config_T,
            typename sensor_class_T>
  void get_flood_sensor_relations(
      std::vector<supla_channel_relation> *relations, e_relation_kind kind,
      unsigned char protocol_version, int func, int parent_channel_func);

 protected:
  virtual unsigned char get_protocol_version(void) = 0;

  virtual void for_each(
      bool any_device,
      std::function<void(supla_abstract_common_channel_properties *, bool *)>
          on_channel_properties) = 0;

  template <typename jsonT, typename sdT>
  void json_to_config(char *config, unsigned _supla_int16_t *config_size,
                      std::function<bool(jsonT *, sdT *)> get_config);

  template <typename configT, typename sensorT>
  void resolve_sensor_identifiers(configT *config);

  void get_config(char *config, unsigned _supla_int16_t *config_size,
                  unsigned char config_type, unsigned _supla_int_t flags,
                  bool resolve_channel_identifiers);

 public:
  supla_abstract_common_channel_properties(void);
  virtual ~supla_abstract_common_channel_properties(void);

  virtual int get_id(void) = 0;
  virtual int get_device_id(void) = 0;
  virtual int get_user_id(void) = 0;
  virtual int get_type(void) = 0;
  virtual int get_func(void) = 0;
  virtual unsigned _supla_int64_t get_flags(void) = 0;
  virtual int get_param1(void) = 0;
  virtual int get_param2(void) = 0;
  virtual int get_param3(void) = 0;
  virtual int get_param4(void) = 0;
  virtual supla_json_config *get_json_config(void) = 0;
  virtual void set_json_config(supla_json_config *config) = 0;
  virtual unsigned char get_channel_number(void) = 0;
  virtual int get_channel_id(unsigned char number);

  std::vector<supla_channel_relation> get_channel_relations(
      e_relation_kind kind);

  void get_channel_relations(std::vector<supla_channel_relation> *relations,
                             e_relation_kind kind);

  int set_user_config(unsigned char config_type,
                      unsigned _supla_int16_t config_size, char *config);
};

#endif /*ABSTRACT_COMMON_CHANNEL_PROPERTIES_H_*/
