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

#ifndef ACCESS_TOKEN_PROVIDERS_H_
#define ACCESS_TOKEN_PROVIDERS_H_

#include <vector>

#include "push/abstract_access_token_provider.h"
#include "push/push_notification_recipient.h"

class supla_access_token_providers {
 private:
  static supla_access_token_providers instance;
  bool initialized;
  std::vector<supla_abstract_access_token_provider *> providers;

 protected:
 public:
  supla_access_token_providers();
  virtual ~supla_access_token_providers(void);
  void initialize(
      void);  // Call only when the server configuration is already loaded
  supla_abstract_access_token_provider *get_provider(_platform_e platform,
                                                     _app_id_e app_id);

  static supla_access_token_providers *get_instance(void);
};

#endif /* ACCESS_TOKEN_PROVIDERS_H_ */
