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

#include "access_token_providers.h"

#include "push/fcm_access_token_provider.h"
#include "svrcfg.h"

// static
supla_access_token_providers supla_access_token_providers::instance;

supla_access_token_providers::supla_access_token_providers(void) {
  initialized = false;
}

supla_access_token_providers::~supla_access_token_providers(void) {
  for (auto it = providers.rbegin(); it != providers.rend(); ++it) {
    delete *it;
  }
}

// static
supla_access_token_providers *supla_access_token_providers::get_instance(void) {
  return &instance;
}

void supla_access_token_providers::initialize(void) {
  if (initialized) {
    return;
  }

  initialized = true;

  const char *key_file_path = scfg_string(CFG_PUSH_FCM_KEY_FILE);
  if (key_file_path && key_file_path[0] != 0) {
    supla_fcm_access_token_provider *provider =
        new supla_fcm_access_token_provider(key_file_path);
    provider->start_service();
    providers.push_back(provider);
  }
}

supla_abstract_access_token_provider *
supla_access_token_providers::get_provider(_platform_e platform,
                                           _app_id_e app_id) {
  for (auto it = providers.rbegin(); it != providers.rend(); ++it) {
    if ((*it)->get_platform() == platform && (*it)->get_app_id() == app_id) {
      return *it;
    }
  }

  return nullptr;
}
