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

#include "call_handler/register_client.h"

#include "client/client.h"
#include "conn/authkey_cache.h"
#include "user/user.h"

using std::shared_ptr;

supla_ch_register_client::supla_ch_register_client(void)
    : supla_ch_abstract_register_client() {}

supla_ch_register_client::~supla_ch_register_client() {}

supla_authkey_cache *supla_ch_register_client::get_authkey_cache(void) {
  return &supla_authkey_cache::get_global_instance();
}

int supla_ch_register_client::get_user_id_by_email(
    const char email[SUPLA_EMAIL_MAXSIZE]) {
  return get_conn_dao()->get_user_id_by_email(email);
}

bool supla_ch_register_client::get_object_id(int user_id,
                                             const char guid[SUPLA_GUID_SIZE],
                                             int *id) {
  return get_client_dao()->get_client_id(user_id, guid, id);
}

bool supla_ch_register_client::get_authkey_hash(
    int id, char authkey_hash[BCRYPT_HASH_MAXSIZE], bool *is_null) {
  return get_client_dao()->get_authkey_hash(id, authkey_hash, is_null);
}

void supla_ch_register_client::superuser_authorize(
    int user_id, const char email[SUPLA_EMAIL_MAXSIZE],
    const char password[SUPLA_PASSWORD_MAXSIZE]) {}

bool supla_ch_register_client::is_superuser_authorized(void) {
  shared_ptr<supla_client> client = get_client().lock();
  return client->is_superuser_authorized();
}

void supla_ch_register_client::revoke_superuser_authorization(void) {
  shared_ptr<supla_client> client = get_client().lock();
  client->revoke_superuser_authorization();
}

void supla_ch_register_client::on_registraction_success(void) {
  shared_ptr<supla_client> client = get_client().lock();

  client->set_id(get_client_id());
  client->set_guid(get_guid());
  client->set_authkey(get_authkey());
  client->set_name(get_name());
  client->set_access_id(get_access_id());
  client->set_user(supla_user::find(get_user_id(), true));
  client->load_config();
  client->set_registered(true);

  supla_user::add_client(client->get_shared_ptr(), get_user_id());
}

int supla_ch_register_client::get_location_count(void) {
  shared_ptr<supla_client> client = get_client().lock();
  return client->get_locations()->count();
}

int supla_ch_register_client::get_channel_count(void) {
  shared_ptr<supla_client> client = get_client().lock();
  return client->get_channels()->count();
}

int supla_ch_register_client::get_channel_group_count(void) {
  shared_ptr<supla_client> client = get_client().lock();
  return client->get_cgroups()->count();
}

int supla_ch_register_client::get_scene_count(void) {
  shared_ptr<supla_client> client = get_client().lock();
  return client->get_scenes()->count();
}

void supla_ch_register_client::remote_update_lists(void) {
  shared_ptr<supla_client> client = get_client().lock();
  client->remote_update_lists();
}
