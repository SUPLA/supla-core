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

#ifndef SUPLA_SET_CG_CHAR_COMMAND_H_
#define SUPLA_SET_CG_CHAR_COMMAND_H_

#include <string>

#include "ipc/abstract_set_cg_char_command.h"

class supla_set_cg_char_command : public supla_abstract_set_cg_char_command {
 protected:
  virtual bool set_cg_char_value(supla_user *user, int group_id, char value);
  explicit supla_set_cg_char_command(
      supla_abstract_ipc_socket_adapter *socket_adapter);
};

#endif /* SUPLA_SET_CG_CHAR_COMMAND_H_ */
