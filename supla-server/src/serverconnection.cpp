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

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <linux/if_link.h>
#include "client/client.h"
#include "database.h"
#include "device/device.h"
#include "log.h"
#include "safearray.h"
#include "serverconnection.h"
#include "srpc.h"
#include "sthread.h"
#include "supla-socket.h"
#include "svrcfg.h"
#include "tools.h"

#define REGISTER_WAIT_TIMEOUT 5
#define REG_NONE 0
#define REG_DEVICE 1
#define REG_CLIENT 2

#define ACTIVITY_TIMEOUT 120
#define ACTIVITY_TIMEOUT_MIN 10
#define ACTIVITY_TIMEOUT_MAX 240

#define INCORRECT_CALL_MAXCOUNT 5

void *serverconnection::reg_pending_arr = NULL;
unsigned int serverconnection::local_ipv4[LOCAL_IPV4_ARRAY_SIZE];

int supla_connection_socket_read(void *buf, int count, void *sc) {
  return ((serverconnection *)sc)->socket_read(buf, count);
}

int supla_connection_socket_write(void *buf, int count, void *sc) {
  return ((serverconnection *)sc)->socket_write(buf, count);
}

void supla_connection_on_remote_call_received(void *_srpc, unsigned int rr_id,
                                              unsigned int call_type, void *sc,
                                              unsigned char proto_version) {
  ((serverconnection *)sc)
      ->on_remote_call_received(_srpc, rr_id, call_type, proto_version);
}

void supla_connection_on_version_error(void *_srpc,
                                       unsigned char remote_version, void *sc) {
  int a;

  srpc_sdc_async_versionerror(_srpc, remote_version);

  for (a = 0; a < 20; a++) srpc_iterate(_srpc);
}

// static
void serverconnection::read_local_ipv4_addresses(void) {
  memset(&serverconnection::local_ipv4, 0, sizeof(local_ipv4));

  struct ifaddrs *ifaddr, *ifa;
  struct sockaddr_in *addr;
  int n = 0;

  if (getifaddrs(&ifaddr) == -1) {
    return;
  }

  for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
    if (ifa && ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET) {
      addr = (struct sockaddr_in *)ifa->ifa_addr;
      serverconnection::local_ipv4[n] = htonl(addr->sin_addr.s_addr);

      if (serverconnection::local_ipv4[n] != 0) {
        n++;
      }
    }
  }

  freeifaddrs(ifaddr);
}

// static
void serverconnection::init(void) {
  serverconnection::read_local_ipv4_addresses();
  serverconnection::reg_pending_arr = safe_array_init();
  cdbase::init();
}

// static
void serverconnection::serverconnection_free(void) {
  cdbase::cdbase_free();
  safe_array_free(serverconnection::reg_pending_arr);
}

// static
int serverconnection::registration_pending_count() {
  return safe_array_count(serverconnection::reg_pending_arr);
}

serverconnection::serverconnection(void *ssd, void *supla_socket,
                                   unsigned int client_ipv4) {
  gettimeofday(&this->init_time, NULL);
  this->client_ipv4 = client_ipv4;
  this->sthread = NULL;
  this->cdptr = NULL;
  this->registered = REG_NONE;
  this->ssd = ssd;
  this->supla_socket = supla_socket;
  this->activity_timeout = ACTIVITY_TIMEOUT;
  this->incorrect_call_counter = 0;

  eh = eh_init();
  eh_add_fd(eh, ssocket_supla_socket_getsfd(supla_socket));

  TsrpcParams srpc_params;
  srpc_params_init(&srpc_params);
  srpc_params.user_params = this;
  srpc_params.data_read = &supla_connection_socket_read;
  srpc_params.data_write = &supla_connection_socket_write;
  srpc_params.on_remote_call_received =
      &supla_connection_on_remote_call_received;
  srpc_params.on_version_error = &supla_connection_on_version_error;
  srpc_params.eh = eh;
  _srpc = srpc_init(&srpc_params);
}

serverconnection::~serverconnection() {
  srpc_free(_srpc);
  eh_free(eh);
  ssocket_supla_socket_free(supla_socket);

  safe_array_remove(serverconnection::reg_pending_arr, this);
  supla_log(LOG_DEBUG, "Connection Finished");
}

int serverconnection::socket_read(void *buf, size_t count) {
  return ssocket_read(ssd, supla_socket, buf, count);
}

int serverconnection::socket_write(const void *buf, size_t count) {
  return ssocket_write(ssd, supla_socket, buf, count);
}

void serverconnection::catch_incorrect_call(unsigned int call_type) {
  incorrect_call_counter++;
  supla_log(LOG_DEBUG, "Incorrect call %i/%i", call_type,
            incorrect_call_counter);

  if (incorrect_call_counter >= INCORRECT_CALL_MAXCOUNT) {
    supla_log(LOG_DEBUG, "The number of incorrect calls has been exceeded.");
    sthread_terminate(sthread);
  }
}

void serverconnection::set_registered(char registered) {
  this->registered = registered;
  safe_array_remove(serverconnection::reg_pending_arr, this);
}

void serverconnection::on_device_reconnect_request(
    void *_srpc, TCS_DeviceReconnectRequest *cs_device_reconnect_request) {
  TSC_DeviceReconnectRequestResult result;
  memset(&result, 0, sizeof(TSC_DeviceReconnectRequestResult));
  result.ResultCode = SUPLA_RESULTCODE_FALSE;

  if (client->is_superuser_authorized()) {
    if (cs_device_reconnect_request &&
        client->getUser()->device_reconnect(
            client->getUser()->getUserID(),
            cs_device_reconnect_request->DeviceID)) {
      result.ResultCode = SUPLA_RESULTCODE_TRUE;
    }

  } else {
    result.ResultCode = SUPLA_RESULTCODE_UNAUTHORIZED;
  }
  srpc_sc_async_device_reconnect_request_result(_srpc, &result);
}

void serverconnection::on_set_channel_function_request(
    TCS_SetChannelFunction *cs_set_channel_function) {
  if (cs_set_channel_function != NULL) {
    client->set_channel_function_request(cs_set_channel_function);
  }
}

void serverconnection::on_set_channel_caption_request(
    TCS_SetChannelCaption *cs_set_channel_caption) {
  if (cs_set_channel_caption != NULL) {
    if (cs_set_channel_caption->CaptionSize > 0) {
      // ! The field in the database is limited to 100 characters !

      if (cs_set_channel_caption->CaptionSize > 101) {
        cs_set_channel_caption->CaptionSize = 101;
      }
    } else {
      cs_set_channel_caption->CaptionSize = 1;
    }

    cs_set_channel_caption->Caption[cs_set_channel_caption->CaptionSize - 1] =
        0;

    client->set_channel_caption_request(cs_set_channel_caption);
  }
}

void serverconnection::on_register_device_request(void *_srpc,
                                                  unsigned int call_type,
                                                  unsigned char proto_version,
                                                  TsrpcReceivedData *rd) {
  switch (call_type) {
    case SUPLA_DS_CALL_REGISTER_DEVICE:

      supla_log(LOG_DEBUG, "SUPLA_DS_CALL_REGISTER_DEVICE");

      if (rd->data.ds_register_device != NULL) {
        TDS_SuplaRegisterDevice_B *register_device_b =
            (TDS_SuplaRegisterDevice_B *)malloc(
                sizeof(TDS_SuplaRegisterDevice_B));

        if (register_device_b != NULL) {
          memset(register_device_b, 0, sizeof(TDS_SuplaRegisterDevice_B));

          register_device_b->LocationID =
              rd->data.ds_register_device->LocationID;
          memcpy(register_device_b->LocationPWD,
                 rd->data.ds_register_device->LocationPWD,
                 SUPLA_LOCATION_PWD_MAXSIZE);
          memcpy(register_device_b->GUID, rd->data.ds_register_device->GUID,
                 SUPLA_GUID_SIZE);
          memcpy(register_device_b->Name, rd->data.ds_register_device->Name,
                 SUPLA_DEVICE_NAME_MAXSIZE);
          memcpy(register_device_b->SoftVer,
                 rd->data.ds_register_device->SoftVer, SUPLA_SOFTVER_MAXSIZE);

          register_device_b->channel_count =
              rd->data.ds_register_device->channel_count;

          for (int c = 0; c < register_device_b->channel_count; c++) {
            register_device_b->channels[c].Number =
                rd->data.ds_register_device->channels[c].Number;
            register_device_b->channels[c].Type =
                rd->data.ds_register_device->channels[c].Type;
            memcpy(register_device_b->channels[c].value,
                   rd->data.ds_register_device->channels[c].value,
                   SUPLA_CHANNELVALUE_SIZE);
          }
        }

        free(rd->data.ds_register_device);
        rd->data.ds_register_device_b = register_device_b;
      }

    /* no break between SUPLA_DS_CALL_REGISTER_DEVICE and
     * SUPLA_DS_CALL_REGISTER_DEVICE_B!!! */
    case SUPLA_DS_CALL_REGISTER_DEVICE_B:

      supla_log(LOG_DEBUG, "SUPLA_DS_CALL_REGISTER_DEVICE_B");

      if (rd->data.ds_register_device_b != NULL) {
        TDS_SuplaRegisterDevice_C *register_device_c =
            (TDS_SuplaRegisterDevice_C *)malloc(
                sizeof(TDS_SuplaRegisterDevice_C));

        if (register_device_c != NULL) {
          memset(register_device_c, 0, sizeof(TDS_SuplaRegisterDevice_C));

          register_device_c->LocationID =
              rd->data.ds_register_device_b->LocationID;
          memcpy(register_device_c->LocationPWD,
                 rd->data.ds_register_device_b->LocationPWD,
                 SUPLA_LOCATION_PWD_MAXSIZE);
          memcpy(register_device_c->GUID, rd->data.ds_register_device_b->GUID,
                 SUPLA_GUID_SIZE);
          memcpy(register_device_c->Name, rd->data.ds_register_device_b->Name,
                 SUPLA_DEVICE_NAME_MAXSIZE);
          memcpy(register_device_c->SoftVer,
                 rd->data.ds_register_device_b->SoftVer, SUPLA_SOFTVER_MAXSIZE);

          register_device_c->channel_count =
              rd->data.ds_register_device_b->channel_count;

          for (int c = 0; c < register_device_c->channel_count; c++) {
            memcpy(&register_device_c->channels[c],
                   &rd->data.ds_register_device_b->channels[c],
                   sizeof(TDS_SuplaDeviceChannel_B));
          }
        }

        free(rd->data.ds_register_device_b);
        rd->data.ds_register_device_c = register_device_c;
      }

    /* no break between SUPLA_DS_CALL_REGISTER_DEVICE_B and
     * SUPLA_DS_CALL_REGISTER_DEVICE_C!!! */
    case SUPLA_DS_CALL_REGISTER_DEVICE_C:

      supla_log(LOG_DEBUG, "SUPLA_DS_CALL_REGISTER_DEVICE_C");

      if (cdptr == NULL && rd->data.ds_register_device_c != NULL) {
        device = new supla_device(this);
        device->retainPtr();

        if (device != NULL) {
          rd->data.ds_register_device_c
              ->LocationPWD[SUPLA_LOCATION_PWD_MAXSIZE - 1] = 0;
          rd->data.ds_register_device_c->Name[SUPLA_DEVICE_NAME_MAXSIZE - 1] =
              0;
          rd->data.ds_register_device_c->SoftVer[SUPLA_SOFTVER_MAXSIZE - 1] = 0;
          rd->data.ds_register_device_c
              ->ServerName[SUPLA_SERVER_NAME_MAXSIZE - 1] = 0;

          if (device->register_device(rd->data.ds_register_device_c, NULL,
                                      proto_version) == 1) {
            set_registered(REG_DEVICE);
          }
        }
      }
      break;
    case SUPLA_DS_CALL_REGISTER_DEVICE_D:

      supla_log(LOG_DEBUG, "SUPLA_DS_CALL_REGISTER_DEVICE_D");

      if (cdptr == NULL && rd->data.ds_register_device_d != NULL) {
        TDS_SuplaRegisterDevice_E *register_device_e =
            (TDS_SuplaRegisterDevice_E *)malloc(
                sizeof(TDS_SuplaRegisterDevice_E));
        if (register_device_e != NULL) {
          memset(register_device_e, 0, sizeof(TDS_SuplaRegisterDevice_E));

          memcpy(register_device_e->Email, rd->data.ds_register_device_d->Email,
                 SUPLA_EMAIL_MAXSIZE);
          memcpy(register_device_e->AuthKey,
                 rd->data.ds_register_device_d->AuthKey, SUPLA_AUTHKEY_SIZE);

          memcpy(register_device_e->GUID, rd->data.ds_register_device_d->GUID,
                 SUPLA_GUID_SIZE);
          memcpy(register_device_e->Name, rd->data.ds_register_device_d->Name,
                 SUPLA_DEVICE_NAME_MAXSIZE);
          memcpy(register_device_e->SoftVer,
                 rd->data.ds_register_device_d->SoftVer, SUPLA_SOFTVER_MAXSIZE);
          memcpy(register_device_e->ServerName,
                 rd->data.ds_register_device_d->ServerName,
                 SUPLA_SERVER_NAME_MAXSIZE);

          register_device_e->channel_count =
              rd->data.ds_register_device_d->channel_count;

          for (int c = 0; c < register_device_e->channel_count; c++) {
            memset(&register_device_e->channels[c], 0,
                   sizeof(TDS_SuplaDeviceChannel_C));
            register_device_e->channels[c].Number =
                rd->data.ds_register_device_d->channels[c].Number;
            register_device_e->channels[c].Type =
                rd->data.ds_register_device_d->channels[c].Type;
            register_device_e->channels[c].FuncList =
                rd->data.ds_register_device_d->channels[c].FuncList;
            register_device_e->channels[c].Default =
                rd->data.ds_register_device_d->channels[c].Default;
            memcpy(register_device_e->channels[c].value,
                   rd->data.ds_register_device_d->channels[c].value,
                   SUPLA_CHANNELVALUE_SIZE);
          }
        }

        free(rd->data.ds_register_device_d);
        rd->data.ds_register_device_e = register_device_e;
      }
    /* no break between SUPLA_DS_CALL_REGISTER_DEVICE_D and
     * SUPLA_DS_CALL_REGISTER_DEVICE_E!!! */
    case SUPLA_DS_CALL_REGISTER_DEVICE_E:
      supla_log(LOG_DEBUG, "SUPLA_DS_CALL_REGISTER_DEVICE_E");

      if (cdptr == NULL && rd->data.ds_register_device_e != NULL) {
        device = new supla_device(this);
        device->retainPtr();

        if (device != NULL) {
          rd->data.ds_register_device_e->Email[SUPLA_EMAIL_MAXSIZE - 1] = 0;
          rd->data.ds_register_device_e->Name[SUPLA_DEVICE_NAME_MAXSIZE - 1] =
              0;
          rd->data.ds_register_device_e->SoftVer[SUPLA_SOFTVER_MAXSIZE - 1] = 0;
          rd->data.ds_register_device_e
              ->ServerName[SUPLA_SERVER_NAME_MAXSIZE - 1] = 0;

          if (device->register_device(NULL, rd->data.ds_register_device_e,
                                      proto_version) == 1) {
            set_registered(REG_DEVICE);
          }
        }
      }
      break;
  }
}

void serverconnection::on_remote_call_received(void *_srpc, unsigned int rr_id,
                                               unsigned int call_type,
                                               unsigned char proto_version) {
  TsrpcReceivedData rd;

  if (srpc_getdata(_srpc, &rd, rr_id) != SUPLA_RESULT_TRUE) return;

  if (call_type == SUPLA_DCS_CALL_GETVERSION) {
    char SoftVer[SUPLA_SOFTVER_MAXSIZE];
    memset(SoftVer, 0, SUPLA_SOFTVER_MAXSIZE);

    snprintf(SoftVer, SUPLA_SOFTVER_MAXSIZE, SERVER_VERSION);
    srpc_sdc_async_getversion_result(_srpc, SoftVer);

    goto end;
  }

  if (registered == REG_NONE) {
    switch (call_type) {
      case SUPLA_DS_CALL_REGISTER_DEVICE:
      case SUPLA_DS_CALL_REGISTER_DEVICE_B:
      case SUPLA_DS_CALL_REGISTER_DEVICE_C:
      case SUPLA_DS_CALL_REGISTER_DEVICE_D:
      case SUPLA_DS_CALL_REGISTER_DEVICE_E:
      case SUPLA_CS_CALL_REGISTER_CLIENT:
      case SUPLA_CS_CALL_REGISTER_CLIENT_B:
      case SUPLA_CS_CALL_REGISTER_CLIENT_C:
      case SUPLA_CS_CALL_REGISTER_CLIENT_D:

        if (srpc_get_proto_version(_srpc) != proto_version) {
          // Adjust version to client/device protocol
          supla_log(LOG_DEBUG, "Adjusting protocol version to %i",
                    proto_version);
          srpc_set_proto_version(_srpc, proto_version);
        }
        break;
    }

    switch (call_type) {
      case SUPLA_DS_CALL_REGISTER_DEVICE:
      case SUPLA_DS_CALL_REGISTER_DEVICE_B:
      case SUPLA_DS_CALL_REGISTER_DEVICE_C:
      case SUPLA_DS_CALL_REGISTER_DEVICE_D:
      case SUPLA_DS_CALL_REGISTER_DEVICE_E:
        on_register_device_request(_srpc, call_type, proto_version, &rd);
        break;

      case SUPLA_CS_CALL_REGISTER_CLIENT:

        supla_log(LOG_DEBUG, "SUPLA_CS_CALL_REGISTER_CLIENT");

        if (rd.data.cs_register_client != NULL) {
          TCS_SuplaRegisterClient_B *register_client_b =
              (TCS_SuplaRegisterClient_B *)malloc(
                  sizeof(TCS_SuplaRegisterClient_B));

          if (register_client_b != NULL) {
            memset(register_client_b, 0, sizeof(TCS_SuplaRegisterClient_B));

            register_client_b->AccessID = rd.data.cs_register_client->AccessID;
            memcpy(register_client_b->AccessIDpwd,
                   rd.data.cs_register_client->AccessIDpwd,
                   SUPLA_ACCESSID_PWD_MAXSIZE);
            memcpy(register_client_b->GUID, rd.data.cs_register_client->GUID,
                   SUPLA_GUID_SIZE);
            memcpy(register_client_b->Name, rd.data.cs_register_client->Name,
                   SUPLA_CLIENT_NAME_MAXSIZE);
            memcpy(register_client_b->SoftVer,
                   rd.data.cs_register_client->SoftVer, SUPLA_SOFTVER_MAXSIZE);
          }

          free(rd.data.cs_register_client);
          rd.data.cs_register_client_b = register_client_b;
        }

      /* no break between SUPLA_CS_CALL_REGISTER_CLIENT and
       * SUPLA_CS_CALL_REGISTER_CLIENT_B!!! */
      case SUPLA_CS_CALL_REGISTER_CLIENT_B:

        supla_log(LOG_DEBUG, "SUPLA_CS_CALL_REGISTER_CLIENT_B");

        if (cdptr == NULL) {
          client = new supla_client(this);
          client->retainPtr();

          if (client != NULL) {
            rd.data.cs_register_client_b
                ->AccessIDpwd[SUPLA_ACCESSID_PWD_MAXSIZE - 1] = 0;
            rd.data.cs_register_client_b->Name[SUPLA_CLIENT_NAME_MAXSIZE - 1] =
                0;
            rd.data.cs_register_client_b->SoftVer[SUPLA_SOFTVER_MAXSIZE - 1] =
                0;
            rd.data.cs_register_client_b
                ->ServerName[SUPLA_SERVER_NAME_MAXSIZE - 1] = 0;

            if (client->register_client(rd.data.cs_register_client_b, NULL,
                                        proto_version) == 1) {
              set_registered(REG_CLIENT);
            }
          }
        }

        break;

      case SUPLA_CS_CALL_REGISTER_CLIENT_C:

        supla_log(LOG_DEBUG, "SUPLA_CS_CALL_REGISTER_CLIENT_C");

        if (rd.data.cs_register_client_c != NULL) {
          TCS_SuplaRegisterClient_D *register_client_d =
              (TCS_SuplaRegisterClient_D *)malloc(
                  sizeof(TCS_SuplaRegisterClient_D));
          if (register_client_d != NULL) {
            memset(register_client_d, 0, sizeof(TCS_SuplaRegisterClient_D));

            memcpy(register_client_d->Email,
                   rd.data.cs_register_client_c->Email, SUPLA_EMAIL_MAXSIZE);
            memcpy(register_client_d->AuthKey,
                   rd.data.cs_register_client_c->AuthKey, SUPLA_AUTHKEY_SIZE);
            memcpy(register_client_d->GUID, rd.data.cs_register_client_c->GUID,
                   SUPLA_GUID_SIZE);
            memcpy(register_client_d->Name, rd.data.cs_register_client_c->Name,
                   SUPLA_CLIENT_NAME_MAXSIZE);
            memcpy(register_client_d->SoftVer,
                   rd.data.cs_register_client_c->SoftVer,
                   SUPLA_SOFTVER_MAXSIZE);
            memcpy(register_client_d->ServerName,
                   rd.data.cs_register_client_c->ServerName,
                   SUPLA_SERVER_NAME_MAXSIZE);

            free(rd.data.cs_register_client_c);
            rd.data.cs_register_client_d = register_client_d;
          }
        }
        /* no break between SUPLA_CS_CALL_REGISTER_CLIENT_C and
         * SUPLA_CS_CALL_REGISTER_CLIENT_D!!! */
      case SUPLA_CS_CALL_REGISTER_CLIENT_D:

        supla_log(LOG_DEBUG, "SUPLA_CS_CALL_REGISTER_CLIENT_D");

        if (cdptr == NULL) {
          client = new supla_client(this);
          client->retainPtr();

          if (client != NULL) {
            rd.data.cs_register_client_d->Email[SUPLA_EMAIL_MAXSIZE - 1] = 0;
            rd.data.cs_register_client_d->Password[SUPLA_PASSWORD_MAXSIZE - 1] =
                0;
            rd.data.cs_register_client_d->Name[SUPLA_CLIENT_NAME_MAXSIZE - 1] =
                0;
            rd.data.cs_register_client_d->SoftVer[SUPLA_SOFTVER_MAXSIZE - 1] =
                0;
            rd.data.cs_register_client_d
                ->ServerName[SUPLA_SERVER_NAME_MAXSIZE - 1] = 0;

            if (client->register_client(NULL, rd.data.cs_register_client_d,
                                        proto_version) == 1) {
              set_registered(REG_CLIENT);
            }
          }
        }
        break;

      default:
        catch_incorrect_call(call_type);
    }

  } else {
    cdptr->updateLastActivity();

    if (call_type == SUPLA_DCS_CALL_SET_ACTIVITY_TIMEOUT &&
        (registered == REG_DEVICE || registered == REG_CLIENT)) {
      if (rd.data.dcs_set_activity_timeout->activity_timeout <
          ACTIVITY_TIMEOUT_MIN)
        rd.data.dcs_set_activity_timeout->activity_timeout =
            ACTIVITY_TIMEOUT_MIN;

      if (rd.data.dcs_set_activity_timeout->activity_timeout >
          ACTIVITY_TIMEOUT_MAX)
        rd.data.dcs_set_activity_timeout->activity_timeout =
            ACTIVITY_TIMEOUT_MAX;

      activity_timeout = rd.data.dcs_set_activity_timeout->activity_timeout;

      TSDC_SuplaSetActivityTimeoutResult result;
      result.activity_timeout = GetActivityTimeout();
      result.min = ACTIVITY_TIMEOUT_MIN;
      result.max = ACTIVITY_TIMEOUT_MAX;

      srpc_dcs_async_set_activity_timeout_result(_srpc, &result);

    } else if (call_type == SUPLA_DCS_CALL_PING_SERVER &&
               (registered == REG_DEVICE || registered == REG_CLIENT)) {
      srpc_sdc_async_ping_server_result(_srpc);

    } else if (call_type == SUPLA_DCS_CALL_GET_USER_LOCALTIME &&
               (registered == REG_DEVICE || registered == REG_CLIENT)) {
      TSDC_UserLocalTimeResult result;
      memset(&result, 0, sizeof(TSDC_UserLocalTimeResult));

      database *db = new database();

      if (!db->connect() ||
          !db->get_user_localtime(cdptr->getUserID(), &result)) {
        memset(&result, 0, sizeof(TSDC_UserLocalTimeResult));
      }

      delete db;

      srpc_sdc_async_get_user_localtime_result(_srpc, &result);

    } else if (call_type == SUPLA_DCS_CALL_GET_REGISTRATION_ENABLED &&
               (registered == REG_DEVICE || registered == REG_CLIENT)) {
      TSDC_RegistrationEnabled reg_en;
      memset(&reg_en, 0, sizeof(TSDC_RegistrationEnabled));

      database *db = new database();

      if (!db->connect() ||
          !db->get_reg_enabled(cdptr->getUserID(), &reg_en.client_timestamp,
                               &reg_en.iodevice_timestamp)) {
        reg_en.client_timestamp = 0;
        reg_en.iodevice_timestamp = 0;
      }

      delete db;

      srpc_sdc_async_get_registration_enabled_result(_srpc, &reg_en);

    } else if (registered == REG_DEVICE) {
      switch (call_type) {
        case SUPLA_DS_CALL_DEVICE_CHANNEL_VALUE_CHANGED:
          if (rd.data.ds_device_channel_value) {
            device->on_device_channel_value_changed(
                rd.data.ds_device_channel_value, NULL, NULL);
          }
          break;
        case SUPLA_DS_CALL_DEVICE_CHANNEL_VALUE_CHANGED_B:
          if (rd.data.ds_device_channel_value_b) {
            device->on_device_channel_value_changed(
                NULL, rd.data.ds_device_channel_value_b, NULL);
          }
          break;
        case SUPLA_DS_CALL_DEVICE_CHANNEL_VALUE_CHANGED_C:
          if (rd.data.ds_device_channel_value_c) {
            device->on_device_channel_value_changed(
                NULL, NULL, rd.data.ds_device_channel_value_c);
          }
          break;

        case SUPLA_DS_CALL_DEVICE_CHANNEL_EXTENDEDVALUE_CHANGED:
          if (rd.data.ds_device_channel_extendedvalue) {
            device->on_device_channel_extendedvalue_changed(
                rd.data.ds_device_channel_extendedvalue);
          }
          break;

        case SUPLA_DS_CALL_CHANNEL_SET_VALUE_RESULT:
          if (rd.data.ds_channel_new_value_result) {
            device->on_channel_set_value_result(
                rd.data.ds_channel_new_value_result);
          }

          break;

        case SUPLA_DS_CALL_GET_FIRMWARE_UPDATE_URL:
          if (rd.data.ds_firmware_update_params) {
            device->get_firmware_update_url(rd.data.ds_firmware_update_params);
          }

          break;

        case SUPLA_DS_CALL_DEVICE_CALCFG_RESULT:
          if (rd.data.ds_device_calcfg_result) {
            device->on_calcfg_result(rd.data.ds_device_calcfg_result);
          }

          break;

        case SUPLA_DSC_CALL_CHANNEL_STATE_RESULT:
          if (rd.data.dsc_channel_state) {
            device->on_channel_state_result(rd.data.dsc_channel_state);
          }
          break;

        case SUPLA_DS_CALL_GET_CHANNEL_FUNCTIONS:
          device->get_channel_functions_request(_srpc);
          break;

        default:
          catch_incorrect_call(call_type);
      }

    } else if (registered == REG_CLIENT) {
      switch (call_type) {
        case SUPLA_CS_CALL_GET_NEXT:
          client->get_next();
          break;
        case SUPLA_CS_CALL_CHANNEL_SET_VALUE:

          supla_log(LOG_DEBUG, "SUPLA_CS_CALL_CHANNEL_SET_VALUE");

          if (rd.data.cs_channel_new_value != NULL) {
            TCS_SuplaChannelNewValue_B *cs_channel_new_value_b =
                (TCS_SuplaChannelNewValue_B *)malloc(
                    sizeof(TCS_SuplaChannelNewValue_B));

            if (cs_channel_new_value_b != NULL) {
              memset(cs_channel_new_value_b, 0,
                     sizeof(TCS_SuplaChannelNewValue_B));
              cs_channel_new_value_b->ChannelId =
                  rd.data.cs_channel_new_value->ChannelId;
              memcpy(cs_channel_new_value_b->value,
                     rd.data.cs_channel_new_value->value,
                     SUPLA_CHANNELVALUE_SIZE);
            }

            free(rd.data.cs_channel_new_value);
            rd.data.cs_channel_new_value_b = cs_channel_new_value_b;
          }
        /* no break between SUPLA_CS_CALL_CHANNEL_SET_VALUE and
         * SUPLA_CS_CALL_CHANNEL_SET_VALUE_B!!! */
        case SUPLA_CS_CALL_CHANNEL_SET_VALUE_B:

          if (rd.data.cs_channel_new_value_b != NULL) {
            supla_log(LOG_DEBUG, "SUPLA_CS_CALL_CHANNEL_SET_VALUE_B");
            client->set_device_channel_new_value(
                rd.data.cs_channel_new_value_b);
          }

          break;

        case SUPLA_CS_CALL_SET_VALUE:

          if (rd.data.cs_new_value != NULL) {
            supla_log(LOG_DEBUG, "SUPLA_CS_CALL_SET_VALUE");
            client->set_new_value(rd.data.cs_new_value);
          }

          break;

        case SUPLA_CS_CALL_OAUTH_TOKEN_REQUEST:
          client->oauth_token_request();
          break;

        case SUPLA_CS_CALL_SUPERUSER_AUTHORIZATION_REQUEST:
          if (rd.data.cs_superuser_authorization_request != NULL) {
            rd.data.cs_superuser_authorization_request
                ->Email[SUPLA_EMAIL_MAXSIZE - 1] = 0;
            rd.data.cs_superuser_authorization_request
                ->Password[SUPLA_PASSWORD_MAXSIZE - 1] = 0;
            client->superuser_authorization_request(
                rd.data.cs_superuser_authorization_request);
          }

          break;

        case SUPLA_CS_CALL_GET_SUPERUSER_AUTHORIZATION_RESULT:
          client->send_superuser_authorization_result(NULL);

          break;

        case SUPLA_CS_CALL_DEVICE_CALCFG_REQUEST:
          if (rd.data.cs_device_calcfg_request != NULL) {
            TCS_DeviceCalCfgRequest_B *cs_device_calcfg_request_b =
                (TCS_DeviceCalCfgRequest_B *)malloc(
                    sizeof(TCS_DeviceCalCfgRequest_B));

            memset(cs_device_calcfg_request_b, 0,
                   sizeof(TCS_DeviceCalCfgRequest_B));

            cs_device_calcfg_request_b->Id =
                rd.data.cs_device_calcfg_request->ChannelID;
            cs_device_calcfg_request_b->Target = SUPLA_TARGET_CHANNEL;
            cs_device_calcfg_request_b->Command =
                rd.data.cs_device_calcfg_request->Command;
            cs_device_calcfg_request_b->DataType =
                rd.data.cs_device_calcfg_request->DataType;
            cs_device_calcfg_request_b->DataSize =
                rd.data.cs_device_calcfg_request->DataSize;
            memcpy(cs_device_calcfg_request_b->Data,
                   rd.data.cs_device_calcfg_request->Data,
                   SUPLA_CALCFG_DATA_MAXSIZE);

            free(rd.data.cs_device_calcfg_request);
            rd.data.cs_device_calcfg_request = NULL;
            rd.data.cs_device_calcfg_request_b = cs_device_calcfg_request_b;
          }
          /* no break between SUPLA_CS_CALL_DEVICE_CALCFG_REQUEST and
           * SUPLA_CS_CALL_DEVICE_CALCFG_REQUEST_B!!! */
        case SUPLA_CS_CALL_DEVICE_CALCFG_REQUEST_B:
          if (rd.data.cs_device_calcfg_request_b != NULL) {
            client->device_calcfg_request(rd.data.cs_device_calcfg_request_b);
          }
          break;
        case SUPLA_CSD_CALL_GET_CHANNEL_STATE:
          if (rd.data.csd_channel_state_request != NULL) {
            client->device_get_channel_state(rd.data.csd_channel_state_request);
          }
          break;
        case SUPLA_CS_CALL_GET_CHANNEL_BASIC_CFG:
          if (rd.data.cs_channel_basic_cfg_request != NULL) {
            client->get_channel_basic_cfg(rd.data.cs_channel_basic_cfg_request);
          }
          break;
        case SUPLA_CS_CALL_SET_CHANNEL_FUNCTION:
          on_set_channel_function_request(rd.data.cs_set_channel_function);
          break;
        case SUPLA_CS_CALL_SET_CHANNEL_CAPTION:
          on_set_channel_caption_request(rd.data.cs_set_channel_caption);
          break;
        case SUPLA_CS_CALL_CLIENTS_RECONNECT_REQUEST:
          if (client->is_superuser_authorized()) {
            client->getUser()->reconnect(EST_CLIENT, false, true);
          } else {
            TSC_ClientsReconnectRequestResult result;
            memset(&result, 0, sizeof(TSC_ClientsReconnectRequestResult));
            result.ResultCode = SUPLA_RESULTCODE_UNAUTHORIZED;
            srpc_sc_async_clients_reconnect_request_result(_srpc, &result);
          }
          break;
        case SUPLA_CS_CALL_SET_REGISTRATION_ENABLED:
          if (rd.data.cs_set_registration_enabled != NULL) {
            TSC_SetRegistrationEnabledResult result;
            memset(&result, 0, sizeof(TSC_SetRegistrationEnabledResult));

            if (client->is_superuser_authorized()) {
              database *db = new database();

              result.ResultCode =
                  db->connect() && db->set_reg_enabled(
                                       cdptr->getUserID(),
                                       rd.data.cs_set_registration_enabled
                                           ->IODeviceRegistrationTimeSec,
                                       rd.data.cs_set_registration_enabled
                                           ->ClientRegistrationTimeSec)
                      ? SUPLA_RESULTCODE_TRUE
                      : SUPLA_RESULTCODE_UNKNOWN_ERROR;

              delete db;
            } else {
              result.ResultCode = SUPLA_RESULTCODE_UNAUTHORIZED;
            }
            srpc_sc_async_set_registration_enabled_result(_srpc, &result);
          }
          break;
        case SUPLA_CS_CALL_DEVICE_RECONNECT_REQUEST:
          on_device_reconnect_request(_srpc,
                                      rd.data.cs_device_reconnect_request);

          break;
        default:
          catch_incorrect_call(call_type);
      }

    } else {
      sthread_terminate(sthread);
    }
  }

end:
  srpc_rd_free(&rd);
}

void serverconnection::execute(void *sthread) {
  this->sthread = sthread;

  int concurrent_registrations_limit =
      scfg_int(CFG_LIMIT_CONCURRENT_REGISTRATIONS);

  if (concurrent_registrations_limit > 0) {
    safe_array_lock(serverconnection::reg_pending_arr);

    if (serverconnection::registration_pending_count() <
        concurrent_registrations_limit) {
      safe_array_add(serverconnection::reg_pending_arr, this);
      concurrent_registrations_limit = 0;
    }

    safe_array_unlock(serverconnection::reg_pending_arr);
  }

  if (concurrent_registrations_limit > 0) {
    for (int a = 0; a < LOCAL_IPV4_ARRAY_SIZE; a++) {
      if (serverconnection::local_ipv4[a] == 0) {
        break;
      } else if (serverconnection::local_ipv4[a] == getClientIpv4()) {
        concurrent_registrations_limit = 0;
        break;
      }
    }
  }

  if (concurrent_registrations_limit > 0) {
    supla_log(LOG_DEBUG, "Connection Dropped");
    sthread_terminate(sthread);
    return;
  }

  if (ssocket_accept_ssl(ssd, supla_socket) != 1) {
    sthread_terminate(sthread);
    return;
  }

  supla_log(LOG_DEBUG, "Connection Started %i, secure=%i", sthread,
            ssocket_is_secure(ssd));

  while (sthread_isterminated(sthread) == 0) {
    eh_wait(eh, registered == REG_NONE ? 1000000 : cdptr->waitTimeUSec());

    if (srpc_iterate(_srpc) == SUPLA_RESULT_FALSE) {
      // supla_log(LOG_DEBUG, "srpc_iterate(_srpc) == SUPLA_RESULT_FALSE");
      break;
    }

    if (registered == REG_NONE) {
      // TERMINATE IF REGISTRATION TIMEOUT
      struct timeval now;
      gettimeofday(&now, NULL);

      if (now.tv_sec - init_time.tv_sec >= REGISTER_WAIT_TIMEOUT) {
        sthread_terminate(sthread);
        supla_log(LOG_DEBUG, "Reg timeout", sthread);
        break;
      }

    } else {
      cdptr->iterate();

      if (cdptr->getActivityDelay() >= GetActivityTimeout()) {
        sthread_terminate(sthread);
        supla_log(LOG_DEBUG, "Activity timeout %i, %i, %i", sthread,
                  cdptr->getActivityDelay(), registered);
        break;
      }
    }
  }

  if (cdptr != NULL) {
    supla_user *user = cdptr->getUser();

    if (user) {
      if (registered == REG_DEVICE) {
        user->moveDeviceToTrash(device);
      } else {
        user->moveClientToTrash(client);
      }
    }

    {
      unsigned long deadlock_counter = 0;

      while (cdptr->ptrCounter() > 1) {
        usleep(100);
        deadlock_counter++;
        if (deadlock_counter > 100000) {
          supla_log(
              LOG_WARNING,
              "Too long waiting time to release the object pointer! %i/%i",
              sthread, cdptr);
          break;
        }
      }
    }

    cdptr->releasePtr();

    if (user) {
      user->emptyTrash();
    } else {
      if (registered == REG_DEVICE) {
        delete device;
      } else {
        delete client;
      }
    }
  }
}

void serverconnection::terminate(void) { sthread_terminate(sthread); }

unsigned int serverconnection::getClientIpv4(void) { return client_ipv4; }

int serverconnection::getClientSD(void) {
  return ssocket_supla_socket_getsfd(supla_socket);
}

void *serverconnection::srpc(void) { return _srpc; }

unsigned char serverconnection::GetActivityTimeout(void) {
  return activity_timeout;
}

unsigned char serverconnection::getProtocolVersion(void) {
  return srpc_get_proto_version(_srpc);
}
