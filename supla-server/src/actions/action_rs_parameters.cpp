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

#include "action_rs_parameters.h"

supla_action_rs_parameters::supla_action_rs_parameters(void)
    : supla_abstract_action_parameters() {
  params = {};
}

supla_action_rs_parameters::supla_action_rs_parameters(char percentage)
    : supla_abstract_action_parameters() {
  params = {};
  params.Percentage = percentage;
}

supla_action_rs_parameters::supla_action_rs_parameters(
    const TAction_RS_Parameters &params)
    : supla_abstract_action_parameters() {
  this->params = params;
}

supla_action_rs_parameters::supla_action_rs_parameters(
    const TAction_RS_Parameters *params)
    : supla_abstract_action_parameters() {
  this->params = {};
  if (params) {
    this->params = *params;
  }
}

supla_action_rs_parameters::~supla_action_rs_parameters(void) {}

bool supla_action_rs_parameters::equal(
    supla_abstract_action_parameters *params) const {
  supla_action_rs_parameters *p =
      dynamic_cast<supla_action_rs_parameters *>(params);
  return p && p->get_percentage() == get_percentage() &&
         p->is_delta() == is_delta();
}

supla_abstract_action_parameters *supla_action_rs_parameters::copy(
    void) const {                                 // NOLINT
  return new supla_action_rs_parameters(params);  // NOLINT
}

char supla_action_rs_parameters::get_percentage(void) const {
  return params.Percentage;
}

void supla_action_rs_parameters::set_percentage(char percentage) {
  params.Percentage = percentage;
}

bool supla_action_rs_parameters::is_delta(void) const {
  return params.Delta > 0;
}

TAction_RS_Parameters supla_action_rs_parameters::get_rs(void) {
  return params;
}
