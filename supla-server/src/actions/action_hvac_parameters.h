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

#ifndef ACTION_HVAC_PARAMETERS_H_
#define ACTION_HVAC_PARAMETERS_H_

#include "actions/abstract_action_parameters.h"
#include "device/value/channel_hvac_value.h"
#include "proto.h"

class supla_action_hvac_parameters : public supla_abstract_action_parameters {
 private:
  TAction_HVAC_Parameters params;

 public:
  supla_action_hvac_parameters(void);
  explicit supla_action_hvac_parameters(const TAction_HVAC_Parameters &params);
  explicit supla_action_hvac_parameters(const TAction_HVAC_Parameters *params);
  virtual ~supla_action_hvac_parameters(void);
  virtual bool equal(supla_abstract_action_parameters *params) const;
  virtual supla_abstract_action_parameters *copy(void) const;  // NOLINT
  void apply_on(supla_channel_hvac_value *value) const;
  unsigned int get_duration_sec(void) const;
  bool is_any_param_set(void) const;
};

#endif /* ACTION_HVAC_PARAMETERS_H_ */
