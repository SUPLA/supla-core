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

#ifndef PUSH_NOTIFICATION_RECIPIENTS_H_
#define PUSH_NOTIFICATION_RECIPIENTS_H_

#include "push/push_notification_recipient.h"

class supla_push_notification_recipients {
 private:
 public:
  supla_push_notification_recipients(void);
  virtual ~supla_push_notification_recipients(void);
  supla_push_notification_recipient* pop(_platform_e platform);
  bool any_recipient_exists(_platform_e platform);
};

#endif /* PUSH_NOTIFICATION_RECIPIENTS_H_ */
