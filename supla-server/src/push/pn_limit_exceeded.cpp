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

#include "pn_limit_exceeded.h"

using std::map;
using std::string;
using std::vector;

supla_pn_limit_exceeded::supla_pn_limit_exceeded(supla_push_notification *push,
                                                 int limit)
    : supla_push_notification(push->get_id()) {
  supla_push_notification::set_localized_body(
      "push_notification_limit_exceeded");
  vector<string> args{std::to_string(limit)};
  supla_push_notification::set_localized_body_args(args);
  supla_push_notification::get_recipients() = push->get_recipients();
  delete push;
}

supla_pn_limit_exceeded::~supla_pn_limit_exceeded(void) {}

void supla_pn_limit_exceeded::set_title(const string &title) {}

void supla_pn_limit_exceeded::set_body(const string &body) {}

void supla_pn_limit_exceeded::set_localized_title(const string &title) {}

void supla_pn_limit_exceeded::set_localized_body(const string &body) {}
void supla_pn_limit_exceeded::set_localized_title_args(
    const vector<string> &args) {}

void supla_pn_limit_exceeded::set_localized_body_args(
    const vector<string> &args) {}

void supla_pn_limit_exceeded::set_sound(int sound) {}

void supla_pn_limit_exceeded::set_replacement_map(
    const map<string, string> &replacement_map) {}
