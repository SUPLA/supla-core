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

#ifndef ACTIONCONFIG_H_
#define ACTIONCONFIG_H_

#include <actions/abstract_action_parameters.h>

#include "abstract_action_config.h"

class supla_action_config : public abstract_action_config {
 private:
  int action_id;
  _subjectType_e subject_type;
  int subject_id;
  int source_device_id;
  int source_channel_id;
  supla_abstract_action_parameters *parameters;

 public:
  supla_action_config(void);
  explicit supla_action_config(supla_action_config *src);
  supla_action_config(const supla_action_config &src);  // NOLINT
  virtual ~supla_action_config(void);

  virtual int get_action_id(void);
  void set_action_id(int action_id);

  virtual _subjectType_e get_subject_type(void);
  void set_subject_type(_subjectType_e subject_type);

  virtual int get_subject_id(void);
  void set_subject_id(int subject_id);

  virtual int get_source_device_id(void);
  void set_source_device_id(int source_device_id);

  virtual int get_source_channel_id(void);
  void set_source_channel_id(int source_channel_id);

  virtual supla_abstract_action_parameters *get_parameters(void);
  void set_parameters(supla_abstract_action_parameters *parameters);

  void apply_json_params(const char *param);

  bool operator==(const supla_action_config &config) const;
  supla_action_config &operator=(const supla_action_config &config);
};

#endif /* ACTIONCONFIG_H_ */
