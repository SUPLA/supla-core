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

#ifndef PUSH_NOTIFICATION_RECIPIENT_H_
#define PUSH_NOTIFICATION_RECIPIENT_H_

#include <string>

class supla_pn_recipient {
 private:
  int client_id;
  int app_id;
  std::string id;
  std::string message_id;
  bool exists;

 public:
  supla_pn_recipient(int client_id, int app_id, const std::string& id);
  virtual ~supla_pn_recipient(void);

  const std::string& get_id(void);
  int get_client_id(void);
  int get_app_id(void);
  void set_message_id(const std::string& message_id);
  std::string get_message_id(void);
  bool is_exists(void);
  void set_exists(bool exists);
};

#endif /* PUSH_NOTIFICATION_RECIPIENT_H_ */
