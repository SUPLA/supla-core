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

#ifndef CLIENT_STATE_H_
#define CLIENT_STATE_H_

#include <string>
#include "common.h"
#include "supla-client-lib/proto.h"

class client_state {
 private:
  uint16_t function;
  std::string payload;
  std::string topic;
  bool enabled;

 public:
  client_state();

  uint16_t getFunction();
  std::string getPayload(int id, std::string caption, bool online);
  std::string getTopic(int id, int type);
  bool getEnabled();

  void setFunction(std::uint16_t value);
  void setPayload(std::string value);
  void setTopic(std::string value);
  void setEnabled(bool value);
};

#endif /* CLIENT_STATE_H_ */
