/*
 Copyright (C) AC SOFTWARE SP. Z O.O.

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.
 */

#ifndef ACTION_MODE_PARAMETERS_H_
#define ACTION_MODE_PARAMETERS_H_

#include "actions/abstract_action_parameters.h"

class supla_action_mode_parameters : public supla_abstract_action_parameters {
 private:
  unsigned char mode;

 public:
  explicit supla_action_mode_parameters(unsigned char mode);
  virtual ~supla_action_mode_parameters(void);
  unsigned char get_mode(void) const;
  virtual bool equal(supla_abstract_action_parameters *params) const;
  virtual supla_abstract_action_parameters *copy(void) const;  // NOLINT
};

#endif /* ACTION_MODE_PARAMETERS_H_ */
