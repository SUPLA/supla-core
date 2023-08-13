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

#include "push_notification.h"

using std::map;
using std::string;
using std::vector;

supla_push_notification::supla_push_notification(void) {
  id = 0;
  sound = 0;
}

supla_push_notification::supla_push_notification(int id) {
  this->id = id;
  sound = 0;
}

supla_push_notification::~supla_push_notification(void) {}

int supla_push_notification::get_id(void) { return id; }

supla_pn_recipients &supla_push_notification::get_recipients(void) {
  return recipients;
}

void supla_push_notification::set_title(const string &title) {
  this->title = title.substr(0, 100);  // Same limit as in the database
  apply_replacement_map();
}

const string &supla_push_notification::get_title(void) { return title; }

void supla_push_notification::set_body(const string &body) {
  this->body = body.substr(0, 255);  // Same limit as in the database
  apply_replacement_map();
}

const string &supla_push_notification::get_body(void) { return body; }

void supla_push_notification::set_localized_title(const string &title) {
  localized_title = title;
  apply_replacement_map();
}

const string &supla_push_notification::get_localized_title(void) {
  return localized_title;
}

void supla_push_notification::set_localized_body(const string &body) {
  localized_body = body;
  apply_replacement_map();
}

const string &supla_push_notification::get_localized_body(void) {
  return localized_body;
}

void supla_push_notification::set_localized_title_args(
    const vector<string> &args) {
  localized_title_args = args;
}

const vector<string> &supla_push_notification::get_localized_title_args() {
  return localized_title_args;
}

void supla_push_notification::set_localized_body_args(
    const vector<string> &args) {
  localized_body_args = args;
}

const vector<string> &supla_push_notification::get_localized_body_args() {
  return localized_body_args;
}

void supla_push_notification::set_sound(int sound) { this->sound = sound; }

int supla_push_notification::get_sound(void) { return sound; }

string supla_push_notification::apply_replacement_map(string str) {
  for (auto it = replacement_map.begin(); it != replacement_map.end(); ++it) {
    string pattern = "{";
    pattern.append(it->first);
    pattern.append("}");

    size_t pos = 0;

    while ((pos = str.find(pattern, pos)) != string::npos) {
      str.replace(pos, pattern.length(), it->second);
      pos += it->second.length();
    }
  }

  return str;
}

void supla_push_notification::apply_replacement_map(void) {
  if (replacement_map.size()) {
    title = apply_replacement_map(title);
    body = apply_replacement_map(body);
    localized_title = apply_replacement_map(localized_title);
    localized_body = apply_replacement_map(localized_body);
  }
}

void supla_push_notification::set_replacement_map(
    map<string, string> *replacement_map) {
  if (replacement_map) {
    this->replacement_map = *replacement_map;
  } else {
    this->replacement_map.clear();
  }

  set_date_time(date_time);  // apply_replacement_map is called at set_date_time
}

void supla_push_notification::set_date_time(const std::string date_time) {
  this->date_time = date_time;
  if (this->date_time.length() == 19) {
    replacement_map["date"] = date_time.substr(0, 10);
    replacement_map["time"] = date_time.substr(11, 8);
    replacement_map["date-time"] = date_time;
  }
  apply_replacement_map();
}

bool supla_push_notification::apply_json(int user_id, cJSON *json) {
  if (!json) {
    return false;
  }

  cJSON *id_json = cJSON_GetObjectItem(json, "id");
  if (id_json) {
    this->id = 0;
    if (cJSON_IsNumber(id_json)) {
      this->id = id_json->valuedouble;
    }
    return this->id;
  }

  cJSON *title_json = cJSON_GetObjectItem(json, "title");
  if (title_json) {
    if (!cJSON_IsString(title_json)) {
      return false;
    }

    set_title(cJSON_GetStringValue(title_json));
  }

  cJSON *body_json = cJSON_GetObjectItem(json, "body");
  if (body_json) {
    if (!cJSON_IsString(body_json)) {
      return false;
    }

    set_body(cJSON_GetStringValue(body_json));
  }

  cJSON *sound_json = cJSON_GetObjectItem(json, "sound");
  if (sound_json) {
    if (!cJSON_IsNumber(sound_json)) {
      return false;
    }

    set_sound(sound_json->valuedouble);
  }

  return recipients.apply_json(user_id, json);
}

bool supla_push_notification::apply_json(int user_id, const char *json) {
  if (!user_id) {
    return false;
  }

  cJSON *json_obj = cJSON_Parse(json);
  if (!json_obj) {
    return false;
  }

  bool result = apply_json(user_id, json_obj);
  cJSON_Delete(json_obj);

  return result;
}
