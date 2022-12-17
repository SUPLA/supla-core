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

#include <conn/abstract_connection_object.h>
#include <string.h>

#include "lck.h"
#include "svrcfg.h"
#include "user.h"

using std::shared_ptr;
using std::weak_ptr;

supla_abstract_connection_object::supla_abstract_connection_object(
    supla_connection *conn) {
  this->user = NULL;
  this->conn = conn;
  this->lck = lck_init();
  this->ID = 0;
  this->registered = false;
  memset(this->guid, 0, SUPLA_GUID_SIZE);
  memset(this->authkey, 0, SUPLA_AUTHKEY_SIZE);

  update_last_activity_time();  // last line / after lck_init
}

supla_abstract_connection_object::~supla_abstract_connection_object() {
  lck_free(this->lck);
}

bool supla_abstract_connection_object::can_reconnect(void) { return true; }

void supla_abstract_connection_object::lock(void) { lck_lock(lck); }

void supla_abstract_connection_object::unlock(void) { lck_unlock(lck); }

void supla_abstract_connection_object::terminate(void) {
  lock();
  if (conn) conn->terminate();
  unlock();
}

bool supla_abstract_connection_object::reconnect() {
  if (can_reconnect()) {
    terminate();
    return true;
  }
  return false;
}

void supla_abstract_connection_object::set_guid(
    const char guid[SUPLA_GUID_SIZE]) {
  lock();
  memcpy(this->guid, guid, SUPLA_GUID_SIZE);
  unlock();
}

void supla_abstract_connection_object::get_guid(char guid[SUPLA_GUID_SIZE]) {
  lock();
  memcpy(guid, this->guid, SUPLA_GUID_SIZE);
  unlock();
}

bool supla_abstract_connection_object::guid_equal(
    const char guid[SUPLA_GUID_SIZE]) {
  lock();
  bool result = memcmp(this->guid, guid, SUPLA_GUID_SIZE) == 0;
  unlock();

  return result;
}

void supla_abstract_connection_object::set_authkey(
    const char authkey[SUPLA_AUTHKEY_SIZE]) {
  if (authkey) {
    lock();
    memcpy(this->authkey, authkey, SUPLA_AUTHKEY_SIZE);
    unlock();
  }
}

void supla_abstract_connection_object::get_authkey(
    char authkey[SUPLA_AUTHKEY_SIZE]) {
  lock();
  memcpy(authkey, this->authkey, SUPLA_AUTHKEY_SIZE);
  unlock();
}

int supla_abstract_connection_object::get_id(void) {
  lock();
  int result = ID;
  unlock();

  return result;
}

void supla_abstract_connection_object::set_id(int ID) {
  lock();
  this->ID = ID;
  unlock();
}

void supla_abstract_connection_object::set_user(supla_user *user) {
  lock();
  this->user = user;
  unlock();
}

supla_user *supla_abstract_connection_object::get_user(void) {
  lock();
  supla_user *result = this->user;
  unlock();

  return result;
}

int supla_abstract_connection_object::get_user_id(void) {
  supla_user *user = get_user();

  if (user != NULL) return user->getUserID();

  return 0;
}

void supla_abstract_connection_object::set_registered(bool registered) {
  lock();
  this->registered = registered;
  unlock();
}

bool supla_abstract_connection_object::is_registered(void) {
  lock();
  bool result = registered;
  unlock();

  return result;
}

void supla_abstract_connection_object::iterate(void) {}

void supla_abstract_connection_object::connection_will_close(void) {}

supla_connection *supla_abstract_connection_object::get_connection(void) {
  return conn;
}

shared_ptr<supla_abstract_connection_object>
supla_abstract_connection_object::get_shared_ptr(void) {
  return conn ? conn->get_object() : nullptr;
}

void supla_abstract_connection_object::update_last_activity_time(void) {
  lock();
  gettimeofday(&last_activity_time, NULL);
  unlock();
}

int supla_abstract_connection_object::get_activity_delay(void) {
  int result;
  struct timeval now;
  gettimeofday(&now, NULL);

  lock();
  result = now.tv_sec - last_activity_time.tv_sec;
  unlock();

  return result;
}

unsigned char supla_abstract_connection_object::get_protocol_version(void) {
  unsigned char result = 0;
  lock();
  if (conn) {
    result = conn->get_protocol_version();
  }
  unlock();
  return result;
}

unsigned _supla_int64_t supla_abstract_connection_object::wait_time_usec() {
  return is_registered() ? 120000000 : 1000000;
}
