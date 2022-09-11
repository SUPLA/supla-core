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

#include "client/call_handler/abstract_register_client.h"

#include <string.h>

using std::weak_ptr;

supla_ch_abstract_register_client::supla_ch_abstract_register_client(void)
    : supla_ch_abstract_register_object() {
  client_dao = nullptr;
}

supla_ch_abstract_register_client::~supla_ch_abstract_register_client() {}

supla_abstract_client_dao *supla_ch_abstract_register_client::get_client_dao(
    void) {
  return client_dao;
}

std::weak_ptr<supla_client> supla_ch_abstract_register_client::get_client(
    void) {
  return client;
}

char supla_ch_abstract_register_client::register_client(
    TCS_SuplaRegisterClient_B *register_client_b,
    TCS_SuplaRegisterClient_D *register_client_d,
    supla_abstract_srpc_adapter *srpc_adapter,
    supla_abstract_db_access_provider *dba,
    supla_abstract_connection_dao *conn_dao,
    supla_abstract_client_dao *client_dao, int client_sd, int client_ipv4,
    unsigned char activity_timeout) {
  int resultcode = SUPLA_RESULTCODE_TEMPORARILY_UNAVAILABLE;
  char result = 0;
  int access_id = 0;

  set_srpc_adapter(srpc_adapter);
  set_dba(dba);
  set_conn_dao(conn_dao);
  this->client_dao = client_dao;
  set_client_sd(client_sd);
  set_client_ipv4(client_ipv4);

  if (register_client_b != nullptr) {
    set_guid(register_client_b->GUID);
    set_name(register_client_b->Name);
    access_id = register_client_b->AccessID;
    set_softver(register_client_b->SoftVer);
  } else {
    set_guid(register_client_d->GUID);
    set_name(register_client_d->Name);
    set_authkey(register_client_d->AuthKey);
    set_softver(register_client_d->SoftVer);
  }

  if (!is_valid_guid()) {
    resultcode = SUPLA_RESULTCODE_GUID_ERROR;

  } else if (register_client_d != nullptr && !is_valid_authkey()) {
    resultcode = SUPLA_RESULTCODE_AUTHKEY_ERROR;

  } else if (register_client_b == nullptr && register_client_d == nullptr) {
    resultcode = SUPLA_RESULTCODE_UNSUPORTED;

  } else {
    if (dba->connect() == true) {
      bool pwd_is_set = false;
      bool accessid_enabled = false;
      bool accessid_active = false;

      if (register_client_b != nullptr &&
          false == client_dao->access_id_auth(
                       access_id, register_client_b->AccessIDpwd,
                       get_user_id_ptr(), &accessid_enabled,
                       &accessid_active)) {
        resultcode = SUPLA_RESULTCODE_BAD_CREDENTIALS;

      } else if (register_client_d != nullptr &&
                 false == authkey_auth(get_guid(), register_client_d->Email,
                                       get_authkey(), get_user_id_ptr())) {
        resultcode = SUPLA_RESULTCODE_BAD_CREDENTIALS;

      } else if (get_user_id() == 0) {
        resultcode = SUPLA_RESULTCODE_BAD_CREDENTIALS;

      } else {
        _supla_int_t _access_id = access_id;

        bool client_enabled = true;
        bool do_update = true;
        bool _accessid_enabled = false;
        bool _accessid_active = false;

        dba->start_transaction();

        // TODO(pzygmunt): move get_client_id higher, and get_client_variables
        // should return a bool. Do the same as in
        // supla_ch_abstract_register_device
        int client_id = client_dao->get_client_variables(
            client_dao->get_client_id(get_user_id(), get_guid()),
            &client_enabled, &_access_id, &_accessid_enabled,
            &_accessid_active);

        if (_accessid_enabled) accessid_enabled = true;
        if (_accessid_active) accessid_active = true;

        pwd_is_set =
            register_client_d != nullptr &&
            strnlen(register_client_d->Password, SUPLA_PASSWORD_MAXSIZE) > 0;

        if (client_id == 0) {
          do_update = false;

          if (pwd_is_set) {
            superuser_authorize(get_user_id(), register_client_d->Email,
                                register_client_d->Password);
          }

          if (false == client_dao->get_client_reg_enabled(get_user_id()) &&
              false == is_superuser_authorized()) {
            dba->rollback();
            resultcode = pwd_is_set ? SUPLA_RESULTCODE_BAD_CREDENTIALS
                                    : SUPLA_RESULTCODE_REGISTRATION_DISABLED;

          } else if (client_dao->get_client_limit_left(get_user_id()) <= 0) {
            dba->rollback();
            resultcode = SUPLA_RESULTCODE_CLIENT_LIMITEXCEEDED;

          } else {
            if (access_id == 0 && register_client_d != nullptr &&
                (db->get_client_count(get_user_id()) == 0 ||
                 is_superuser_authorized())) {
              AccessID = db->get_access_id(UserID, true, true);

              if (AccessID > 0) {
                accessid_enabled = true;
                accessid_active = true;
              } else {
                accessid_enabled = false;
                AccessID = db->get_access_id(UserID, false, true);
                if (AccessID > 0) {
                  accessid_active = true;
                } else {
                  accessid_active = false;
                  AccessID = db->get_access_id(UserID, false, false);
                }
              }
            }

            ClientID = db->add_client(AccessID, GUID, AuthKey, Name,
                                      get_connection()->get_client_ipv4(),
                                      SoftVer, proto_version, UserID);

            if (ClientID == 0) {
              // something goes wrong
              db->rollback();

            } else {
              client_enabled = true;

              if (AccessID == 0) {
                _accessid_enabled = false;
                _accessid_active = false;
                AccessID = db->get_client_access_id(
                    ClientID, &_accessid_enabled, &_accessid_active);

                if (AccessID) {
                  if (_accessid_enabled) {
                    accessid_enabled = true;
                  }
                  if (_accessid_active) {
                    accessid_active = true;
                  }
                }
              }
            }
          }

        } else if (_AccessID > 0 && register_client_d != nullptr) {
          AccessID = _AccessID;
        }

        if (ClientID != 0) {
          if (!client_enabled) {
            db->rollback();
            resultcode = SUPLA_RESULTCODE_CLIENT_DISABLED;

          } else {
            if (do_update) {
              if (AccessID == 0 && pwd_is_set) {
                if (!is_superuser_authorized()) {
                  superuser_authorize(UserID, register_client_d->Email,
                                      register_client_d->Password, nullptr);
                }

                if (is_superuser_authorized()) {
                  AccessID = db->get_access_id(UserID, true, true);

                  if (AccessID) {
                    accessid_enabled = true;
                    accessid_active = true;
                  }
                }
              }

              if (false ==
                  db->update_client(ClientID, AccessID, AuthKey, Name,
                                    get_connection()->get_client_ipv4(),
                                    SoftVer, proto_version)) {
                // something goes wrong
                ClientID = 0;
                db->rollback();
              }
            }

            if (ClientID) {
              db->commit();

              if (AccessID == 0) {
                resultcode = pwd_is_set
                                 ? SUPLA_RESULTCODE_BAD_CREDENTIALS
                                 : SUPLA_RESULTCODE_ACCESSID_NOT_ASSIGNED;

              } else if (!accessid_enabled) {
                resultcode = SUPLA_RESULTCODE_ACCESSID_DISABLED;

              } else if (!accessid_active) {
                resultcode = SUPLA_RESULTCODE_ACCESSID_INACTIVE;

              } else {
                set_id(ClientID);
                setName(Name);
                setAccessID(AccessID);

                // Set the user before loading config
                set_user(supla_user::add_client(get_shared_ptr(), UserID));

                loadConfig();

                resultcode = SUPLA_RESULTCODE_TRUE;
                result = 1;
              }
            }
          }
        }
      }
      dba->disconnect();
    }
  }

  revoke_superuser_authorization();

  if (proto_version >= 17) {
    TSC_SuplaRegisterClientResult_C srcr = {};
    srcr.result_code = resultcode;
    srcr.ClientID = get_id();
    srcr.activity_timeout = get_connection()->get_activity_timeout();
    srcr.version_min = SUPLA_PROTO_VERSION_MIN;
    srcr.version = SUPLA_PROTO_VERSION;
    srcr.LocationCount = locations->count();
    srcr.ChannelCount = channels->count();
    srcr.ChannelGroupCount = cgroups->count();

    struct timeval now;
    gettimeofday(&now, nullptr);
    srcr.serverUnixTimestamp = now.tv_sec;

    srpc_sc_async_registerclient_result_c(
        get_connection()->get_srpc_adapter()->get_srpc(), &srcr);
  } else if (proto_version >= 9) {
    TSC_SuplaRegisterClientResult_B srcr = {};
    srcr.result_code = resultcode;
    srcr.ClientID = get_id();
    srcr.activity_timeout = get_connection()->get_activity_timeout();
    srcr.version_min = SUPLA_PROTO_VERSION_MIN;
    srcr.version = SUPLA_PROTO_VERSION;
    srcr.LocationCount = locations->count();
    srcr.ChannelCount = channels->count();
    srcr.ChannelGroupCount = cgroups->count();
    srpc_sc_async_registerclient_result_b(
        get_connection()->get_srpc_adapter()->get_srpc(), &srcr);
  } else {
    TSC_SuplaRegisterClientResult srcr = {};
    srcr.result_code = resultcode;
    srcr.ClientID = get_id();
    srcr.activity_timeout = get_connection()->get_activity_timeout();
    srcr.version_min = SUPLA_PROTO_VERSION_MIN;
    srcr.version = SUPLA_PROTO_VERSION;
    srcr.LocationCount = locations->count();
    srcr.ChannelCount = channels->count();
    srpc_sc_async_registerclient_result(
        get_connection()->get_srpc_adapter()->get_srpc(), &srcr);
  }

  // !After srpc_async_registerclient_result
  if (resultcode == SUPLA_RESULTCODE_TRUE) {
    remote_update_lists();
    supla_log(LOG_INFO, "Client registered. ClientSD: %i Protocol Version: %i",
              get_connection()->get_client_sd(),
              get_connection()->get_protocol_version());
  } else {
    usleep(2000000);
  }

  return result;
}
