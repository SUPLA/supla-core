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

#ifndef SUPLA_ABSTRACT_SET_RGBW_COMMAND_H_
#define SUPLA_ABSTRACT_SET_RGBW_COMMAND_H_

#include <string>

#include "ipc/abstract_ipc_command.h"
#include "user.h"

class supla_abstract_set_rgbw_command : public supla_abstract_ipc_command {
 private:
  bool random_color;

 protected:
  virtual void on_command_match(const char *params);
  virtual const std::string get_command_name(void);
  virtual bool set_channel_rgbw_value(supla_user *user, int device_id,
                                      int channel_id, int color,
                                      char color_brightness, char brightness,
                                      char on_off,
                                      const char *alexa_correlation_token,
                                      const char *google_request_id) = 0;

 public:
  explicit supla_abstract_set_rgbw_command(
      supla_abstract_ipc_socket_adapter *socket_adapter, bool random_color);
  static int get_random_color(void);
};

#endif /* SUPLA_ABSTRACT_SET_RGBW_COMMAND_H_ */
