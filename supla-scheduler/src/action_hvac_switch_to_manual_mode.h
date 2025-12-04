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

#ifndef ACTION_HVAC_SWITCH_TO_MANUAL_MODE_H_
#define ACTION_HVAC_SWITCH_TO_MANUAL_MODE_H_

#include "action_hvac_switch_to.h"

class s_worker_action_switch_to_manual_mode : public s_worker_action_switch_to {
 protected:
  virtual bool to_program_mode(void);

 public:
  explicit s_worker_action_switch_to_manual_mode(s_abstract_worker *worker);
};

#endif /*ACTION_HVAC_SWITCH_TO_MANUAL_MODE_H_*/
