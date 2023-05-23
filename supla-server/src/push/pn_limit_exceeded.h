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

#ifndef PUSH_NOTIFICATION_LIMIT_EXCEEDED_H_
#define PUSH_NOTIFICATION_LIMIT_EXCEEDED_H_

#include <map>
#include <string>
#include <vector>

#include "push/push_notification.h"

class supla_pn_limit_exceeded : public supla_push_notification {
 public:
  explicit supla_pn_limit_exceeded(supla_push_notification *push, int limit);
  virtual ~supla_pn_limit_exceeded(void);

  virtual void set_title(const std::string &title);
  virtual void set_body(const std::string &body);
  virtual void set_localized_title(const std::string &title);
  virtual void set_localized_body(const std::string &body);
  virtual void set_localized_title_args(const std::vector<std::string> &args);
  virtual void set_localized_body_args(const std::vector<std::string> &args);
  virtual void set_sound(int sound);
  virtual void set_replacement_map(
      const std::map<std::string, std::string> &replacement_map);
};

#endif /* PUSH_NOTIFICATION_LIMIT_EXCEEDED_H_ */
