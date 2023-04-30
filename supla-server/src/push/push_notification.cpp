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

using std::string;

supla_push_notification::supla_push_notification(void) {
  id = 0;
  sound = 0;
}

supla_push_notification::supla_push_notification(int id) {
  this->id = id;
  sound = 0;
}

supla_push_notification::~supla_push_notification(void) {}

int supla_push_notification::get_id(void) { return id; };

supla_pn_recipients &supla_push_notification::get_recipients(void) {
  return recipients;
}

void supla_push_notification::set_recipients(
    const supla_pn_recipients &recipients) {
  this->recipients = recipients;
}

void supla_push_notification::set_title(const string &title) {
  this->title = title;
}

const string &supla_push_notification::get_title(void) { return title; }

void supla_push_notification::set_body(const string &body) {
  this->body = body;
}

const string &supla_push_notification::get_body(void) { return body; }

void supla_push_notification::set_sound(int sound) { this->sound = sound; }

int supla_push_notification::get_sound(void) { return sound; }
