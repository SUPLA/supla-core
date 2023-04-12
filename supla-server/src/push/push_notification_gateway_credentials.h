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

#ifndef PUSH_NOTIFICATION_GATEWAY_CREDENTIALS_H_
#define PUSH_NOTIFICATION_GATEWAY_CREDENTIALS_H_

#include <string>

class supla_push_notification_gateway_credentials {
 public:
  supla_push_notification_gateway_credentials(void);
  virtual ~supla_push_notification_gateway_credentials(void);

  virtual std::string get_token(void) = 0;
  virtual std::string get_url(void) = 0;
};

#endif /* PUSH_NOTIFICATION_GATEWAY_CREDENTIALS_H_ */
