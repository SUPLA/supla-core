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
  this->title = title;
  apply_replacement_map();
}

const string &supla_push_notification::get_title(void) { return title; }

void supla_push_notification::set_body(const string &body) {
  this->body = body;
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
    const map<string, string> &replacement_map) {
  this->replacement_map = replacement_map;
  apply_replacement_map();
}

