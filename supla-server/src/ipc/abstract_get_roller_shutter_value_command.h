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

#ifndef SUPLA_ABSTRACT_GET_ROLLER_SHUTTER_VALUE_COMMAND_H_
#define SUPLA_ABSTRACT_GET_ROLLER_SHUTTER_VALUE_COMMAND_H_

#include <string>

#include "device/value/channel_rs_value.h"
#include "ipc/abstract_ipc_command.h"
#include "jsonconfig/json_config.h"

class supla_abstract_get_roller_shutter_value_command
    : public supla_abstract_ipc_command {
 protected:
  virtual void on_command_match(const char *params);
  virtual supla_channel_rs_value *get_channel_value(int user_id, int device_id,
                                                    int channel_id) = 0;

 public:
  explicit supla_abstract_get_roller_shutter_value_command(
      supla_abstract_ipc_socket_adapter *socket_adapter);
  virtual const std::string get_command_name(void);
};

#endif /* SUPLA_ABSTRACT_GET_ROLLER_SHUTTER_VALUE_COMMAND_H_ */
