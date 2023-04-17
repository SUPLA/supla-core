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

enum _platform_e {
  platform_unknown,
  platform_ios,
  platform_android,
};

enum _app_id_e {
  app_supla = 0,
  app_nice = 1,
  app_comelit = 2,
};

class supla_push_notification_recipient {
 private:
  _platform_e platform;
  _app_id_e app_id;
  std::string id;

 public:
  supla_push_notification_recipient(_platform_e type, _app_id_e app_id,
                                    const std::string& id);
  virtual ~supla_push_notification_recipient(void);

  const std::string& get_id(void);
  _platform_e get_platform(void);
  _app_id_e get_app_id(void);
};

#endif /* PUSH_NOTIFICATION_RECIPIENT_H_ */
