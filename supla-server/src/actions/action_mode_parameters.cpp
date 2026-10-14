/*
 Copyright (C) AC SOFTWARE SP. Z O.O.

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.
 */

#include "actions/action_mode_parameters.h"

supla_action_mode_parameters::supla_action_mode_parameters(unsigned char mode)
    : mode(mode) {}

supla_action_mode_parameters::~supla_action_mode_parameters(void) {}

unsigned char supla_action_mode_parameters::get_mode(void) const {
  return mode;
}

bool supla_action_mode_parameters::equal(
    supla_abstract_action_parameters *params) const {
  supla_action_mode_parameters *other =
      dynamic_cast<supla_action_mode_parameters *>(params);
  return other && other->mode == mode;
}

supla_abstract_action_parameters *supla_action_mode_parameters::copy(
    void) const {  // NOLINT
  return new supla_action_mode_parameters(mode);
}
