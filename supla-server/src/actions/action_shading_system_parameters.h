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

#ifndef ACTION_RS_PARAMETERS_H_
#define ACTION_RS_PARAMETERS_H_

#include "actions/abstract_action_parameters.h"
#include "json/cJSON.h"
#include "proto.h"

class supla_action_shading_system_parameters
    : public supla_abstract_action_parameters {
 private:
  TAction_ShadingSystem_Parameters params;
  char percentage_to_position(char percentage) const;
  char clamp(char percentage) const;
  char add_delta(char current, char delta) const;

 public:
  supla_action_shading_system_parameters(void);
  supla_action_shading_system_parameters(char percentage, char tilt,
                                         unsigned char flags);
  explicit supla_action_shading_system_parameters(
      const TAction_ShadingSystem_Parameters &params);
  explicit supla_action_shading_system_parameters(
      const TAction_ShadingSystem_Parameters *params);
  virtual ~supla_action_shading_system_parameters(void);
  virtual bool equal(supla_abstract_action_parameters *params) const;
  virtual supla_abstract_action_parameters *copy(void) const;  // NOLINT

  char get_percentage(void) const;
  char get_tilt(void) const;
  void set_percentage(char percentage);
  void set_tilt(char tilt);
  unsigned char get_flags(void) const;
  void set_flags(unsigned char flags);

  TAction_ShadingSystem_Parameters get_params(void);
  bool apply_on_value(int action, char value[SUPLA_CHANNELVALUE_SIZE], int func,
                      unsigned _supla_int64_t flags) const;

  static supla_action_shading_system_parameters *create_from_json(cJSON *root);
};

#endif /* ACTION_RS_PARAMETERS_H_ */
