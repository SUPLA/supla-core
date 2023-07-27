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
#include "proto.h"

class supla_action_rs_parameters : public supla_abstract_action_parameters {
 private:
  TAction_RS_Parameters params;

 public:
  supla_action_rs_parameters(void);
  explicit supla_action_rs_parameters(char percentage);
  explicit supla_action_rs_parameters(const TAction_RS_Parameters &params);
  explicit supla_action_rs_parameters(const TAction_RS_Parameters *params);
  virtual ~supla_action_rs_parameters(void);
  virtual bool equal(supla_abstract_action_parameters *params) const;
  virtual supla_abstract_action_parameters *copy(void);  // NOLINT

  char get_percentage(void) const;
  void set_percentage(char percentage);
  bool is_delta(void) const;
  TAction_RS_Parameters get_rs(void);
};

#endif /* ACTION_RS_PARAMETERS_H_ */
