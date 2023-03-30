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

#include "alexa_request2.h"

using std::string;

supla_alexa_request2::supla_alexa_request2(
    const supla_caller &caller, int user_id, int device_id, int channel_id,
    supla_asynctask_queue *queue, supla_abstract_asynctask_thread_pool *pool,
    supla_abstract_channel_property_getter *property_getter,
    supla_amazon_alexa_credentials2 *credentials)
    : supla_asynctask_http_request(caller, user_id, device_id, channel_id,
                                   queue, pool, property_getter) {
  this->credentials = credentials;
}

supla_amazon_alexa_credentials2 *supla_alexa_request2::get_credentials(void) {
  return credentials;
}

string supla_alexa_request2::get_zulu_time(void) {
  lock();
  string result = zulu_time;
  unlock();

  return result;
}

void supla_alexa_request2::set_zulu_time(const string &zulu_time) {
  lock();
  this->zulu_time = zulu_time;
  unlock();
}

string supla_alexa_request2::get_message_id(void) {
  lock();
  string result = message_id;
  unlock();

  return result;
}

void supla_alexa_request2::set_message_id(const string &message_id) {
  lock();
  this->message_id = message_id;
  unlock();
}
