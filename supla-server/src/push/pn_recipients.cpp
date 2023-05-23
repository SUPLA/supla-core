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

#include "pn_recipients.h"

#include "db/db_access_provider.h"
#include "log.h"
#include "push/pn_recipient_dao.h"

using std::map;
using std::vector;

supla_pn_recipients::supla_pn_recipients(void) {}

supla_pn_recipients::~supla_pn_recipients(void) { clear(); }

void supla_pn_recipients::add(supla_pn_recipient* recipient,
                              _platform_e platform) {
  auto mit = recipients.find(platform);
  if (mit == recipients.end()) {
    recipients[platform] = vector<supla_pn_recipient*>();
  }

  recipients[platform].push_back(recipient);
}

supla_pn_recipient* supla_pn_recipients::get(_platform_e platform,
                                             size_t index) {
  auto mit = recipients.find(platform);

  if (mit != recipients.end() && index < mit->second.size()) {
    return mit->second.at(index);
  }

  return nullptr;
}

supla_pn_recipient* supla_pn_recipients::get(size_t index) {
  for (auto mit = recipients.begin(); mit != recipients.end(); ++mit) {
    if (index >= mit->second.size()) {
      index -= mit->second.size();
      continue;
    }

    return mit->second.at(index);
  }

  return nullptr;
}

size_t supla_pn_recipients::count(_platform_e platform) {
  auto mit = recipients.find(platform);
  if (mit != recipients.end()) {
    return mit->second.size();
  }

  return 0;
}

size_t supla_pn_recipients::total_count(void) {
  size_t result = 0;
  for (auto mit = recipients.begin(); mit != recipients.end(); ++mit) {
    result += mit->second.size();
  }
  return result;
}

void supla_pn_recipients::clear(void) {
  for (auto mit = recipients.begin(); mit != recipients.end(); ++mit) {
    for (auto it = mit->second.begin(); it != mit->second.end(); ++it) {
      delete *it;
    }
  }

  recipients.clear();
}

bool supla_pn_recipients::get_ids(cJSON* json, vector<int>* ids) {
  int size = cJSON_GetArraySize(json);

  ids->reserve(size);

  for (int a = 0; a < size; a++) {
    cJSON* id_json = cJSON_GetArrayItem(json, a);
    if (!id_json || !cJSON_IsNumber(id_json)) {
      return false;
    }

    ids->push_back(id_json->valuedouble);
  }

  return true;
}

bool supla_pn_recipients::apply_json(int user_id, cJSON* json) {
  cJSON* recipients_json = cJSON_GetObjectItem(json, "recipients");
  if (!recipients_json) {
    return false;
  }

  cJSON* aids_json = cJSON_GetObjectItem(recipients_json, "aids");
  cJSON* cids_json = cJSON_GetObjectItem(recipients_json, "cids");

  if (!aids_json && !cids_json) {
    return false;
  }

  vector<int> aids;
  if (!get_ids(aids_json, &aids)) {
    return false;
  }

  vector<int> cids;
  if (!get_ids(cids_json, &cids)) {
    return false;
  }

  supla_db_access_provider dba;
  supla_pn_recipient_dao dao(&dba);

  dao.get_recipients(user_id, aids, cids, this);

  return true;
}

supla_pn_recipients& supla_pn_recipients::operator=(
    const supla_pn_recipients& src) {
  clear();

  for (auto mit = src.recipients.begin(); mit != src.recipients.end(); ++mit) {
    for (auto it = mit->second.begin(); it != mit->second.end(); ++it) {
      add(new supla_pn_recipient(*it), mit->first);
    }
  }

  return *this;
}
