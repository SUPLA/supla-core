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

#include "lck.h"
#include "log.h"
#include "srpc.h"
#include "supla-client.h"
#include "supla-socket.h"

typedef struct {
  void *ssd;
  void *eh;
  void *srpc;
  void *lck;

  int client_id;

  struct timeval last_ping;
  struct timeval last_call_sent;
  struct timeval last_call_recv;
  char connected;
  char registered;
  int server_activity_timeout;

  TSuplaClientCfg cfg;
} TSuplaClientData;

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdint.h>

// http://stackoverflow.com/questions/10905892/equivalent-of-gettimeday-for-windows
int gettimeofday(struct timeval *tp, struct timezone *tzp) {
  // Note: some broken versions only have 8 trailing zero's, the correct epoch
  // has 9 trailing zero's
  static const uint64_t EPOCH = ((uint64_t)116444736000000000ULL);

  SYSTEMTIME system_time;
  FILETIME file_time;
  uint64_t time;

  GetSystemTime(&system_time);
  SystemTimeToFileTime(&system_time, &file_time);
  time = ((uint64_t)file_time.dwLowDateTime);
  time += ((uint64_t)file_time.dwHighDateTime) << 32;

  tp->tv_sec = (long)((time - EPOCH) / 10000000L);
  tp->tv_usec = (long)(system_time.wMilliseconds * 1000);
  return 0;
}
#endif

int supla_client_socket_read(void *buf, int count, void *scd) {
  return ssocket_read(((TSuplaClientData *)scd)->ssd, NULL, buf, count);
}

int supla_client_socket_write(void *buf, int count, void *scd) {
  return ssocket_write(((TSuplaClientData *)scd)->ssd, NULL, buf, count);
}

void supla_client_before_async_call(void *_srpc,
                                    unsigned _supla_int_t call_type,
                                    void *_scd) {
  TSuplaClientData *scd = (TSuplaClientData *)_scd;
  gettimeofday(&scd->last_call_sent, NULL);
}

void supla_client_on_min_version_required(void *_srpc,
                                          unsigned _supla_int_t call_type,
                                          unsigned char min_version,
                                          void *_scd) {
  TSuplaClientData *scd = (TSuplaClientData *)_scd;

  if (scd->cfg.cb_on_min_version_required) {
    scd->cfg.cb_on_min_version_required(scd, scd->cfg.user_data, call_type,
                                        min_version);
  }
}

void supla_client_on_version_error(TSuplaClientData *scd,
                                   TSDC_SuplaVersionError *version_error) {
  supla_log(LOG_ERR,
            "Protocol version error. Server doesn't support this client. "
            "S:%d-%d/C:%d",
            version_error->server_version_min, version_error->server_version,
            SUPLA_PROTO_VERSION);

  if (scd->cfg.cb_on_versionerror) {
    scd->cfg.cb_on_versionerror(scd, scd->cfg.user_data, SUPLA_PROTO_VERSION,
                                version_error->server_version_min,
                                version_error->server_version);
  }

  supla_client_disconnect(scd);
}

char supla_client_registered(void *_suplaclient) {
  TSuplaClientData *scd = (TSuplaClientData *)_suplaclient;
  char result;
  lck_lock(scd->lck);
  result = scd->registered;
  lck_unlock(scd->lck);

  return result;
}

void supla_client_set_registered(void *_suplaclient, char registered) {
  TSuplaClientData *scd = (TSuplaClientData *)_suplaclient;

  lck_lock(scd->lck);
  scd->registered = registered;
  lck_unlock(scd->lck);
}

void supla_client_on_register_result(
    TSuplaClientData *scd,
    TSC_SuplaRegisterClientResult_B *register_client_result) {
  if (register_client_result->result_code == SUPLA_RESULTCODE_TRUE) {
    supla_client_set_registered(scd, 1);

    scd->server_activity_timeout = register_client_result->activity_timeout;
    scd->client_id = register_client_result->ClientID;

    supla_log(LOG_DEBUG, "Registered.");

    if (scd->cfg.cb_on_registered)
      scd->cfg.cb_on_registered(scd, scd->cfg.user_data,
                                register_client_result);

  } else {
    switch (register_client_result->result_code) {
      case SUPLA_RESULTCODE_BAD_CREDENTIALS:
        supla_log(LOG_ERR, "Bad credentials!");
        break;

      case SUPLA_RESULTCODE_TEMPORARILY_UNAVAILABLE:
        supla_log(LOG_NOTICE, "Temporarily unavailable!");
        break;

      case SUPLA_RESULTCODE_ACCESSID_DISABLED:
        supla_log(LOG_NOTICE, "Access Identifier is disabled!");
        break;

      case SUPLA_RESULTCODE_CLIENT_DISABLED:
        supla_log(LOG_NOTICE, "Client is disabled!");
        break;

      case SUPLA_RESULTCODE_CLIENT_LIMITEXCEEDED:
        supla_log(LOG_NOTICE, "Client limit exceeded!");
        break;

      case SUPLA_RESULTCODE_GUID_ERROR:
        supla_log(LOG_NOTICE, "Incorrect client GUID!");
        break;

      case SUPLA_RESULTCODE_REGISTRATION_DISABLED:
        supla_log(LOG_NOTICE, "Registration disabled!");
        break;

      case SUPLA_RESULTCODE_ACCESSID_NOT_ASSIGNED:
        supla_log(LOG_NOTICE, "Access Identifier not assigned!");
        break;

      case SUPLA_RESULTCODE_AUTHKEY_ERROR:
        supla_log(LOG_NOTICE, "Incorrect AuthKey!");
        break;
    }

    if (scd->cfg.cb_on_registererror)
      scd->cfg.cb_on_registererror(scd, scd->cfg.user_data,
                                   register_client_result->result_code);

    supla_client_disconnect(scd);
  }
}

void supla_client_set_str(char *str, unsigned int *size, unsigned int max) {
  if (*size > max) *size = max;

  if (*size > 0)
    str[(*size) - 1] = 0;
  else
    str[0] = 0;
}

void supla_client_location_update(TSuplaClientData *scd,
                                  TSC_SuplaLocation *location, char gn) {
  supla_client_set_str(location->Caption, &location->CaptionSize,
                       SUPLA_LOCATION_CAPTION_MAXSIZE);

  if (scd->cfg.cb_location_update)
    scd->cfg.cb_location_update(scd, scd->cfg.user_data, location);

  if (gn == 1) srpc_cs_async_get_next(scd->srpc);
}

void supla_client_locationpack_update(TSuplaClientData *scd,
                                      TSC_SuplaLocationPack *pack) {
  int a;

  for (a = 0; a < pack->count; a++)
    supla_client_location_update(scd, &pack->items[a], 0);

  srpc_cs_async_get_next(scd->srpc);
}

void supla_client_channel_update_b(TSuplaClientData *scd,
                                   TSC_SuplaChannel_B *channel, char gn) {
  supla_client_set_str(channel->Caption, &channel->CaptionSize,
                       SUPLA_CHANNEL_CAPTION_MAXSIZE);

  if (scd->cfg.cb_channel_update)
    scd->cfg.cb_channel_update(scd, scd->cfg.user_data, channel);

  if (gn == 1) srpc_cs_async_get_next(scd->srpc);
}

void supla_client_channelgroup_update(TSuplaClientData *scd,
                                      TSC_SuplaChannelGroup *channel_group,
                                      char gn) {
  supla_client_set_str(channel_group->Caption, &channel_group->CaptionSize,
                       SUPLA_CHANNELGROUP_CAPTION_MAXSIZE);

  if (scd->cfg.cb_channelgroup_update)
    scd->cfg.cb_channelgroup_update(scd, scd->cfg.user_data, channel_group);

  if (gn == 1) srpc_cs_async_get_next(scd->srpc);
}

void supla_client_channelgroup_pack_update(TSuplaClientData *scd,
                                           TSC_SuplaChannelGroupPack *pack) {
  int a;

  for (a = 0; a < pack->count; a++)
    supla_client_channelgroup_update(scd, &pack->items[a], 0);

  srpc_cs_async_get_next(scd->srpc);
}

void supla_client_channelgroup_relation_update(
    TSuplaClientData *scd, TSC_SuplaChannelGroupRelation *channelgroup_relation,
    char gn) {
  if (scd->cfg.cb_channelgroup_relation_update)
    scd->cfg.cb_channelgroup_relation_update(scd, scd->cfg.user_data,
                                             channelgroup_relation);

  if (gn == 1) srpc_cs_async_get_next(scd->srpc);
}

void supla_client_channelgroup_relation_pack_update(
    TSuplaClientData *scd, TSC_SuplaChannelGroupRelationPack *pack) {
  int a;

  for (a = 0; a < pack->count; a++)
    supla_client_channelgroup_relation_update(scd, &pack->items[a], 0);

  srpc_cs_async_get_next(scd->srpc);
}

void supla_client_channel_value_update(TSuplaClientData *scd,
                                       TSC_SuplaChannelValue *channel_value,
                                       char gn) {
  if (scd->cfg.cb_channel_value_update)
    scd->cfg.cb_channel_value_update(scd, scd->cfg.user_data, channel_value);

  if (gn == 1) {
    srpc_cs_async_get_next(scd->srpc);
  }
}

void supla_client_channel_extendedvalue_update(
    TSuplaClientData *scd,
    TSC_SuplaChannelExtendedValue *channel_extendedvalue) {
  if (scd->cfg.cb_channel_extendedvalue_update)
    scd->cfg.cb_channel_extendedvalue_update(scd, scd->cfg.user_data,
                                             channel_extendedvalue);
}

void supla_client_channelvalue_pack_update(TSuplaClientData *scd,
                                           TSC_SuplaChannelValuePack *pack) {
  int a;

  if (pack && pack->count <= SUPLA_CHANNELVALUE_PACK_MAXCOUNT) {
    for (a = 0; a < pack->count; a++) {
      supla_client_channel_value_update(scd, &pack->items[a], 0);
    }
  }

  srpc_cs_async_get_next(scd->srpc);
}

void supla_client_channelextendedvalue_pack_update(
    TSuplaClientData *scd, TSC_SuplaChannelExtendedValuePack *pack) {
  TSC_SuplaChannelExtendedValue ev;
  int n = 0;
  int offset = 0;
  int min_size =
      sizeof(TSC_SuplaChannelExtendedValue) - SUPLA_CHANNELEXTENDEDVALUE_SIZE;

  if (pack != NULL) {
    while (pack->pack_size - offset >= min_size && n < pack->count) {
      memset(&ev, 0, sizeof(TSC_SuplaChannelExtendedValue));
      memcpy(&ev, &pack->pack[offset], min_size);
      offset += min_size;

      if (ev.value.size > 0 && ev.value.type != 0 &&
          pack->pack_size - offset >= ev.value.size) {
        memcpy(ev.value.value, &pack->pack[offset], ev.value.size);

        offset += ev.value.size;
        supla_client_channel_extendedvalue_update(scd, &ev);
      }

      n++;
    }
  }

  srpc_cs_async_get_next(scd->srpc);
}

void supla_client_channel_a2b(TSC_SuplaChannel *a, TSC_SuplaChannel_B *b) {
  b->EOL = a->EOL;
  b->Id = a->Id;
  b->LocationID = a->LocationID;
  b->Func = a->Func;
  b->AltIcon = 0;
  b->Flags = 0;
  b->ProtocolVersion = 0;
  b->online = a->online;
  memcpy(&b->value, &a->value, sizeof(TSuplaChannelValue));
  b->CaptionSize = a->CaptionSize;
  memcpy(b->Caption, a->Caption, SUPLA_CHANNEL_CAPTION_MAXSIZE);
}

void supla_client_channel_update(TSuplaClientData *scd,
                                 TSC_SuplaChannel *channel, char gn) {
  TSC_SuplaChannel_B channel_b;
  memset(&channel_b, 0, sizeof(TSC_SuplaChannel_B));

  supla_client_channel_a2b(channel, &channel_b);
  supla_client_channel_update_b(scd, &channel_b, gn);
}

void supla_client_channelpack_update(TSuplaClientData *scd,
                                     TSC_SuplaChannelPack *pack) {
  int a;

  for (a = 0; a < pack->count; a++)
    supla_client_channel_update(scd, &pack->items[a], 0);

  srpc_cs_async_get_next(scd->srpc);
}

void supla_client_channelpack_update_b(TSuplaClientData *scd,
                                       TSC_SuplaChannelPack_B *pack) {
  int a;

  for (a = 0; a < pack->count; a++)
    supla_client_channel_update_b(scd, &pack->items[a], 0);

  srpc_cs_async_get_next(scd->srpc);
}

void supla_client_on_event(TSuplaClientData *scd, TSC_SuplaEvent *event) {
  supla_client_set_str(event->SenderName, &event->SenderNameSize,
                       SUPLA_SENDER_NAME_MAXSIZE);

  if (scd->cfg.cb_on_event)
    scd->cfg.cb_on_event(scd, scd->cfg.user_data, event);
}

void supla_client_on_remote_call_received(void *_srpc, unsigned int rr_id,
                                          unsigned int call_type, void *_scd,
                                          unsigned char proto_version) {
  TsrpcReceivedData rd;
  char result;
  TSuplaClientData *scd = (TSuplaClientData *)_scd;

  gettimeofday(&scd->last_call_recv, NULL);

  supla_log(LOG_DEBUG, "on_remote_call_received: %i", call_type);

  if (SUPLA_RESULT_TRUE == (result = srpc_getdata(_srpc, &rd, 0))) {
    switch (rd.call_type) {
      case SUPLA_SDC_CALL_VERSIONERROR:
        supla_client_on_version_error(scd, rd.data.sdc_version_error);
        break;
      case SUPLA_SC_CALL_REGISTER_CLIENT_RESULT:

        if (rd.data.sc_register_client_result == NULL) {
          break;
        }

        {
          TSC_SuplaRegisterClientResult_B *sc_register_client_result_b =
              (TSC_SuplaRegisterClientResult_B *)malloc(
                  sizeof(TSC_SuplaRegisterClientResult_B));

          if (sc_register_client_result_b == NULL) {
            break;
          } else {
            sc_register_client_result_b->result_code =
                rd.data.sc_register_client_result->result_code;

            sc_register_client_result_b->ClientID =
                rd.data.sc_register_client_result->ClientID;

            sc_register_client_result_b->LocationCount =
                rd.data.sc_register_client_result->LocationCount;

            sc_register_client_result_b->ChannelCount =
                rd.data.sc_register_client_result->ChannelCount;

            sc_register_client_result_b->ChannelGroupCount = 0;
            sc_register_client_result_b->Flags = 0;

            sc_register_client_result_b->activity_timeout =
                rd.data.sc_register_client_result->activity_timeout;

            sc_register_client_result_b->version =
                rd.data.sc_register_client_result->version;

            sc_register_client_result_b->version_min =
                rd.data.sc_register_client_result->version_min;

            free(rd.data.sc_register_client_result);
            rd.data.sc_register_client_result_b = sc_register_client_result_b;
          }
        }

      /* no break between SUPLA_SC_CALL_REGISTER_CLIENT_RESULT and
       * SUPLA_SC_CALL_REGISTER_CLIENT_RESULT_B!!! */
      case SUPLA_SC_CALL_REGISTER_CLIENT_RESULT_B:
        supla_client_on_register_result(scd,
                                        rd.data.sc_register_client_result_b);
        break;
      case SUPLA_SC_CALL_LOCATION_UPDATE:
        supla_client_location_update(scd, rd.data.sc_location, 1);
        break;
      case SUPLA_SC_CALL_LOCATIONPACK_UPDATE:
        supla_client_locationpack_update(scd, rd.data.sc_location_pack);
        break;
      case SUPLA_SC_CALL_CHANNEL_UPDATE:
        supla_client_channel_update(scd, rd.data.sc_channel, 1);
        break;
      case SUPLA_SC_CALL_CHANNELPACK_UPDATE:
        supla_client_channelpack_update(scd, rd.data.sc_channel_pack);
        break;
      case SUPLA_SC_CALL_CHANNELPACK_UPDATE_B:
        supla_client_channelpack_update_b(scd, rd.data.sc_channel_pack_b);
        break;
      case SUPLA_SC_CALL_CHANNEL_VALUE_UPDATE:
        supla_client_channel_value_update(scd, rd.data.sc_channel_value, 1);
        break;
      case SUPLA_SC_CALL_CHANNELGROUP_PACK_UPDATE:
        supla_client_channelgroup_pack_update(scd,
                                              rd.data.sc_channelgroup_pack);
        break;
      case SUPLA_SC_CALL_CHANNELGROUP_RELATION_PACK_UPDATE:
        supla_client_channelgroup_relation_pack_update(
            scd, rd.data.sc_channelgroup_relation_pack);
        break;
      case SUPLA_SC_CALL_CHANNELVALUE_PACK_UPDATE:
        supla_client_channelvalue_pack_update(scd,
                                              rd.data.sc_channelvalue_pack);
        break;
      case SUPLA_SC_CALL_CHANNELEXTENDEDVALUE_PACK_UPDATE:
        supla_client_channelextendedvalue_pack_update(
            scd, rd.data.sc_channelextendedvalue_pack);
        break;
      case SUPLA_SC_CALL_EVENT:
        supla_client_on_event(scd, rd.data.sc_event);
        break;
      case SUPLA_SDC_CALL_GET_REGISTRATION_ENABLED_RESULT:

        if (scd->cfg.cb_on_registration_enabled)
          scd->cfg.cb_on_registration_enabled(scd, scd->cfg.user_data,
                                              rd.data.sdc_reg_enabled);
        break;
      case SUPLA_SC_CALL_OAUTH_TOKEN_REQUEST_RESULT:
        if (scd->cfg.cb_on_oauth_token_request_result &&
            rd.data.sc_oauth_tokenrequest_result->Token.TokenSize <=
                SUPLA_OAUTH_TOKEN_MAXSIZE)
          scd->cfg.cb_on_oauth_token_request_result(
              scd, scd->cfg.user_data, rd.data.sc_oauth_tokenrequest_result);
        break;
    }

    srpc_rd_free(&rd);

  } else if (result == (char)SUPLA_RESULT_DATA_ERROR) {
    supla_log(LOG_DEBUG, "DATA ERROR!");
  }
}

void supla_client_cfginit(TSuplaClientCfg *sclient_cfg) {
  memset(sclient_cfg, 0, sizeof(TSuplaClientCfg));
  sclient_cfg->tcp_port = 2015;
  sclient_cfg->ssl_port = 2016;
  sclient_cfg->ssl_enabled = 1;
#ifdef _WIN32
  sclient_cfg->iterate_wait_usec = 1000;
#else
  sclient_cfg->iterate_wait_usec = 1000000;
#endif
}

void *supla_client_init(TSuplaClientCfg *sclient_cfg) {
  TSuplaClientData *scd = malloc(sizeof(TSuplaClientData));
  memset(scd, 0, sizeof(TSuplaClientData));
  memcpy(&scd->cfg, sclient_cfg, sizeof(TSuplaClientCfg));

  scd->lck = lck_init();
  scd->cfg.Email[SUPLA_EMAIL_MAXSIZE - 1] = 0;
  scd->cfg.AccessIDpwd[SUPLA_ACCESSID_PWD_MAXSIZE - 1] = 0;
  scd->cfg.Name[SUPLA_CLIENT_NAME_MAXSIZE - 1] = 0;
  scd->cfg.host = NULL;

  if (sclient_cfg->host != NULL && strlen(sclient_cfg->host) > 0) {
#ifdef _WIN32
    scd->cfg.host = _strdup(sclient_cfg->host);
#else
    scd->cfg.host = strdup(sclient_cfg->host);
#endif
  }

  scd->ssd = ssocket_client_init(
      scd->cfg.host,
      scd->cfg.ssl_enabled == 1 ? scd->cfg.ssl_port : scd->cfg.tcp_port,
      scd->cfg.ssl_enabled == 1);

  return scd;
}

void supla_client_clean(void *_suplaclient) {
  void *eh;
  void *srpc;

  TSuplaClientData *suplaclient = (TSuplaClientData *)_suplaclient;

  if (suplaclient) {
    if (suplaclient->eh) {
      eh = suplaclient->eh;
      suplaclient->eh = NULL;
      eh_free(eh);
    }

    if (suplaclient->srpc) {
      srpc = suplaclient->srpc;
      suplaclient->srpc = NULL;
      srpc_free(srpc);
    }
  }
}

void supla_client_free(void *_suplaclient) {
  if (_suplaclient != NULL) {
    supla_client_disconnect(_suplaclient);
    supla_client_clean(_suplaclient);

    TSuplaClientData *scd = (TSuplaClientData *)_suplaclient;

    if (scd->cfg.host) free(scd->cfg.host);

    ssocket_free(scd->ssd);
    lck_free(scd->lck);

    free(_suplaclient);
  }
}

int supla_client_get_id(void *_suplaclient) {
  return ((TSuplaClientData *)_suplaclient)->client_id;
}

char supla_client_connected(void *_suplaclient) {
  return ((TSuplaClientData *)_suplaclient)->connected == 1;
}

void supla_client_disconnect(void *_suplaclient) {
  TSuplaClientData *suplaclient = (TSuplaClientData *)_suplaclient;

  if (supla_client_connected(_suplaclient)) {
    suplaclient->connected = 0;

    supla_client_set_registered(_suplaclient, 0);

    ssocket_supla_socket__close(suplaclient->ssd);

    if (suplaclient->cfg.cb_on_disconnected)
      suplaclient->cfg.cb_on_disconnected(_suplaclient,
                                          suplaclient->cfg.user_data);
  }
}

char supla_client_connect(void *_suplaclient) {
  TSuplaClientData *suplaclient = (TSuplaClientData *)_suplaclient;
  supla_client_disconnect(_suplaclient);

  supla_client_clean(_suplaclient);

  int err = 0;

  if (ssocket_client_connect(suplaclient->ssd, NULL, &err) == 1) {
    suplaclient->eh = eh_init();
    TsrpcParams srpc_params;
    srpc_params_init(&srpc_params);
    srpc_params.user_params = _suplaclient;
    srpc_params.data_read = &supla_client_socket_read;
    srpc_params.data_write = &supla_client_socket_write;
    srpc_params.on_remote_call_received = &supla_client_on_remote_call_received;
    srpc_params.before_async_call = &supla_client_before_async_call;
    srpc_params.on_min_version_required = &supla_client_on_min_version_required;
    srpc_params.eh = suplaclient->eh;
    suplaclient->srpc = srpc_init(&srpc_params);

    if (suplaclient->cfg.protocol_version > 0) {
      srpc_set_proto_version(suplaclient->srpc,
                             suplaclient->cfg.protocol_version);
    }

    eh_add_fd(suplaclient->eh, ssocket_get_fd(suplaclient->ssd));
    suplaclient->connected = 1;

    supla_client_set_registered(_suplaclient, 0);

    if (suplaclient->cfg.cb_on_connected)
      suplaclient->cfg.cb_on_connected(_suplaclient,
                                       suplaclient->cfg.user_data);

    return 1;

  } else {
    if (suplaclient->cfg.cb_on_connerror)
      suplaclient->cfg.cb_on_connerror(_suplaclient, suplaclient->cfg.user_data,
                                       err);
  }

  return 0;
}

void supla_client_register(TSuplaClientData *suplaclient) {
  if (suplaclient->cfg.cb_on_registering)
    suplaclient->cfg.cb_on_registering(suplaclient, suplaclient->cfg.user_data);

  supla_log(LOG_DEBUG, "EMAIL: %s", suplaclient->cfg.Email);

  if (strnlen(suplaclient->cfg.Email, SUPLA_EMAIL_MAXSIZE) > 0 &&
      srpc_call_allowed(suplaclient->srpc, SUPLA_CS_CALL_REGISTER_CLIENT_C)) {
    TCS_SuplaRegisterClient_C src;
    memset(&src, 0, sizeof(TCS_SuplaRegisterClient_C));

#ifdef _WIN32
    _snprintf_s(src.Email, SUPLA_EMAIL_MAXSIZE, _TRUNCATE, "%s",
                suplaclient->cfg.Email);
    _snprintf_s(src.Name, SUPLA_CLIENT_NAME_MAXSIZE, _TRUNCATE, "%s",
                suplaclient->cfg.Name);
    _snprintf_s(src.SoftVer, SUPLA_SOFTVER_MAXSIZE, _TRUNCATE, "%s",
                suplaclient->cfg.SoftVer);
    _snprintf_s(src.ServerName, SUPLA_SERVER_NAME_MAXSIZE, _TRUNCATE, "%s",
                suplaclient->cfg.host);
#else
    snprintf(src.Email, SUPLA_EMAIL_MAXSIZE, "%s", suplaclient->cfg.Email);
    snprintf(src.Name, SUPLA_CLIENT_NAME_MAXSIZE, "%s", suplaclient->cfg.Name);
    snprintf(src.SoftVer, SUPLA_SOFTVER_MAXSIZE, "%s",
             suplaclient->cfg.SoftVer);
    snprintf(src.ServerName, SUPLA_SERVER_NAME_MAXSIZE, "%s",
             suplaclient->cfg.host);
#endif

    memcpy(src.AuthKey, suplaclient->cfg.AuthKey, SUPLA_AUTHKEY_SIZE);
    memcpy(src.GUID, suplaclient->cfg.clientGUID, SUPLA_GUID_SIZE);
    srpc_cs_async_registerclient_c(suplaclient->srpc, &src);

  } else if (srpc_call_allowed(suplaclient->srpc,
                               SUPLA_CS_CALL_REGISTER_CLIENT_B)) {
    TCS_SuplaRegisterClient_B src;
    memset(&src, 0, sizeof(TCS_SuplaRegisterClient_B));

    src.AccessID = suplaclient->cfg.AccessID;

#ifdef _WIN32
    _snprintf_s(src.AccessIDpwd, SUPLA_ACCESSID_PWD_MAXSIZE, _TRUNCATE, "%s",
                suplaclient->cfg.AccessIDpwd);
    _snprintf_s(src.Name, SUPLA_CLIENT_NAME_MAXSIZE, _TRUNCATE, "%s",
                suplaclient->cfg.Name);
    _snprintf_s(src.SoftVer, SUPLA_SOFTVER_MAXSIZE, _TRUNCATE, "%s",
                suplaclient->cfg.SoftVer);
    _snprintf_s(src.ServerName, SUPLA_SERVER_NAME_MAXSIZE, _TRUNCATE, "%s",
                suplaclient->cfg.host);
#else
    snprintf(src.AccessIDpwd, SUPLA_ACCESSID_PWD_MAXSIZE, "%s",
             suplaclient->cfg.AccessIDpwd);
    snprintf(src.Name, SUPLA_CLIENT_NAME_MAXSIZE, "%s", suplaclient->cfg.Name);
    snprintf(src.SoftVer, SUPLA_SOFTVER_MAXSIZE, "%s",
             suplaclient->cfg.SoftVer);
    snprintf(src.ServerName, SUPLA_SERVER_NAME_MAXSIZE, "%s",
             suplaclient->cfg.host);
#endif

    memcpy(src.GUID, suplaclient->cfg.clientGUID, SUPLA_GUID_SIZE);
    srpc_cs_async_registerclient_b(suplaclient->srpc, &src);

  } else if (srpc_call_allowed(suplaclient->srpc,
                               SUPLA_CS_CALL_REGISTER_CLIENT)) {
    TCS_SuplaRegisterClient src;
    memset(&src, 0, sizeof(TCS_SuplaRegisterClient));

    src.AccessID = suplaclient->cfg.AccessID;

#ifdef _WIN32
    _snprintf_s(src.AccessIDpwd, SUPLA_ACCESSID_PWD_MAXSIZE, _TRUNCATE, "%s",
                suplaclient->cfg.AccessIDpwd);
    _snprintf_s(src.Name, SUPLA_CLIENT_NAME_MAXSIZE, _TRUNCATE, "%s",
                suplaclient->cfg.Name);
    _snprintf_s(src.SoftVer, SUPLA_SOFTVER_MAXSIZE, _TRUNCATE, "%s",
                suplaclient->cfg.SoftVer);
#else
    snprintf(src.AccessIDpwd, SUPLA_ACCESSID_PWD_MAXSIZE, "%s",
             suplaclient->cfg.AccessIDpwd);
    snprintf(src.Name, SUPLA_CLIENT_NAME_MAXSIZE, "%s", suplaclient->cfg.Name);
    snprintf(src.SoftVer, SUPLA_SOFTVER_MAXSIZE, "%s",
             suplaclient->cfg.SoftVer);
#endif

    memcpy(src.GUID, suplaclient->cfg.clientGUID, SUPLA_GUID_SIZE);
    srpc_cs_async_registerclient(suplaclient->srpc, &src);
  }
}

void supla_client_ping(TSuplaClientData *suplaclient) {
  struct timeval now;

  if (suplaclient->server_activity_timeout > 0) {
    gettimeofday(&now, NULL);

    int server_activity_timeout = suplaclient->server_activity_timeout - 10;

    if (now.tv_sec - suplaclient->last_ping.tv_sec >= 2 &&
        ((now.tv_sec - suplaclient->last_call_sent.tv_sec) >=
             server_activity_timeout ||
         (now.tv_sec - suplaclient->last_call_recv.tv_sec) >=
             server_activity_timeout)) {
      gettimeofday(&suplaclient->last_ping, NULL);
      srpc_dcs_async_ping_server(suplaclient->srpc);
    }
  }
}

char supla_client_iterate(void *_suplaclient, int wait_usec) {
  TSuplaClientData *suplaclient = (TSuplaClientData *)_suplaclient;

  if (supla_client_connected(_suplaclient) == 0) return 0;

  if (supla_client_registered(_suplaclient) == 0) {
    supla_client_set_registered(_suplaclient, -1);
    supla_client_register(suplaclient);

  } else if (supla_client_registered(_suplaclient) == 1) {
    supla_client_ping(suplaclient);
  }

  if (suplaclient->srpc != NULL &&
      srpc_iterate(suplaclient->srpc) == SUPLA_RESULT_FALSE) {
    supla_client_disconnect(_suplaclient);
    return 0;
  }

  if (supla_client_connected(_suplaclient) == 1 && suplaclient->eh != NULL) {
    eh_wait(suplaclient->eh, wait_usec);
  }

  return 1;
}

void *supla_client_get_userdata(void *_suplaclient) {
  TSuplaClientData *suplaclient = (TSuplaClientData *)_suplaclient;
  return suplaclient->cfg.user_data;
}

char supla_client_open(void *_suplaclient, int ID, char group, char open) {
  TSuplaClientData *suplaclient = (TSuplaClientData *)_suplaclient;
  char result = 0;

  lck_lock(suplaclient->lck);
  if (supla_client_registered(_suplaclient) == 1) {
    if (srpc_get_proto_version(suplaclient->srpc) >= 9) {
      TCS_SuplaNewValue value;
      memset(&value, 0, sizeof(TCS_SuplaNewValue));
      value.Id = ID;
      value.Target = group > 0 ? SUPLA_NEW_VALUE_TARGET_GROUP
                               : SUPLA_NEW_VALUE_TARGET_CHANNEL;
      value.value[0] = open;
      result = srpc_cs_async_set_value(suplaclient->srpc, &value) ==
                       SUPLA_RESULT_FALSE
                   ? 0
                   : 1;
    } else {
      TCS_SuplaChannelNewValue_B value;
      memset(&value, 0, sizeof(TCS_SuplaChannelNewValue_B));
      value.ChannelId = ID;
      value.value[0] = open;
      result = srpc_cs_async_set_channel_value_b(suplaclient->srpc, &value) ==
                       SUPLA_RESULT_FALSE
                   ? 0
                   : 1;
    }
  }
  lck_unlock(suplaclient->lck);

  return result;
}

void _supla_client_set_rgbw(char *value, int color, char color_brightness,
                            char brightness) {
  value[0] = brightness;
  value[1] = color_brightness;
  value[2] = (char)((color & 0x000000FF));        // BLUE
  value[3] = (char)((color & 0x0000FF00) >> 8);   // GREEN
  value[4] = (char)((color & 0x00FF0000) >> 16);  // RED
}
char supla_client_set_rgbw(void *_suplaclient, int ID, char group, int color,
                           char color_brightness, char brightness) {
  TSuplaClientData *suplaclient = (TSuplaClientData *)_suplaclient;
  char result = 0;

  lck_lock(suplaclient->lck);
  if (supla_client_registered(_suplaclient) == 1) {
    if (srpc_get_proto_version(suplaclient->srpc) >= 9) {
      TCS_SuplaNewValue value;
      memset(&value, 0, sizeof(TCS_SuplaNewValue));
      _supla_client_set_rgbw(value.value, color, color_brightness, brightness);
      value.Id = ID;
      value.Target = group > 0 ? SUPLA_NEW_VALUE_TARGET_GROUP
                               : SUPLA_NEW_VALUE_TARGET_CHANNEL;
      result = srpc_cs_async_set_value(suplaclient->srpc, &value) ==
                       SUPLA_RESULT_FALSE
                   ? 0
                   : 1;
    } else {
      TCS_SuplaChannelNewValue_B value;
      memset(&value, 0, sizeof(TCS_SuplaChannelNewValue_B));
      _supla_client_set_rgbw(value.value, color, color_brightness, brightness);
      value.ChannelId = ID;

      result = srpc_cs_async_set_channel_value_b(suplaclient->srpc, &value) ==
                       SUPLA_RESULT_FALSE
                   ? 0
                   : 1;
    }
  }
  lck_unlock(suplaclient->lck);

  return result;
}

char supla_client_set_dimmer(void *_suplaclient, int ID, char group,
                             char brightness) {
  return supla_client_set_rgbw(_suplaclient, ID, group, 0, 0, brightness);
}

char supla_client_get_registration_enabled(void *_suplaclient) {
  return srpc_dcs_async_get_registration_enabled(
      ((TSuplaClientData *)_suplaclient)->srpc);
}

unsigned char supla_client_get_proto_version(void *_suplaclient) {
  return srpc_get_proto_version(((TSuplaClientData *)_suplaclient)->srpc);
}

char supla_client_oauth_token_request(void *_suplaclient) {
  return srpc_cs_async_oauth_token_request(
             ((TSuplaClientData *)_suplaclient)->srpc) > 0;
}
