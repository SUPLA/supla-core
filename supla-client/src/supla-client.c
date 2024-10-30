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

#include "supla-client.h"

#include <stdlib.h>
#include <string.h>

#include "lck.h"
#include "log.h"
#include "srpc.h"
#include "supla-socket.h"
#include "tools.h"

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
  int server_time_diff;

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

void supla_client_before_async_call(void *_srpc, unsigned _supla_int_t call_id,
                                    void *_scd) {
  TSuplaClientData *scd = (TSuplaClientData *)_scd;
  gettimeofday(&scd->last_call_sent, NULL);
}

void supla_client_on_min_version_required(void *_srpc,
                                          unsigned _supla_int_t call_id,
                                          unsigned char min_version,
                                          void *_scd) {
  TSuplaClientData *scd = (TSuplaClientData *)_scd;

  if (scd->cfg.cb_on_min_version_required) {
    scd->cfg.cb_on_min_version_required(scd, scd->cfg.user_data, call_id,
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
    TSC_SuplaRegisterClientResult_D *register_client_result) {
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

void supla_client__set_str(char *str, unsigned short *size,
                           unsigned short max) {
  unsigned int isize = *size;
  supla_client_set_str(str, &isize, max);
  *size = isize;
}

void supla_client_location_update(TSuplaClientData *scd,
                                  TSC_SuplaLocation *location, char gn) {
  supla_client_set_str(location->Caption, &location->CaptionSize,
                       SUPLA_CAPTION_MAXSIZE);

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

void supla_client_channel_update_e(TSuplaClientData *scd,
                                   TSC_SuplaChannel_E *channel, char gn) {
  supla_client_set_str(channel->Caption, &channel->CaptionSize,
                       SUPLA_CHANNEL_CAPTION_MAXSIZE);

  if (scd->cfg.cb_channel_update)
    scd->cfg.cb_channel_update(scd, scd->cfg.user_data, channel);

  if (gn == 1) srpc_cs_async_get_next(scd->srpc);
}

void supla_client_channelgroup_update(TSuplaClientData *scd,
                                      TSC_SuplaChannelGroup_B *channel_group,
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

  for (a = 0; a < pack->count; a++) {
    TSC_SuplaChannelGroup_B channel_group;

    channel_group.EOL = pack->items[a].EOL;
    channel_group.Id = pack->items[a].Id;
    channel_group.LocationID = pack->items[a].LocationID;
    channel_group.Func = pack->items[a].Func;
    channel_group.AltIcon = pack->items[a].AltIcon;
    channel_group.UserIcon = 0;
    channel_group.Flags = pack->items[a].Flags;
    channel_group.CaptionSize = pack->items[a].CaptionSize;
    memcpy(channel_group.Caption, pack->items[a].Caption,
           SUPLA_CHANNELGROUP_CAPTION_MAXSIZE);

    supla_client_channelgroup_update(scd, &channel_group, 0);
  }

  srpc_cs_async_get_next(scd->srpc);
}

void supla_client_channelgroup_pack_update_b(
    TSuplaClientData *scd, TSC_SuplaChannelGroupPack_B *pack) {
  int a;

  for (a = 0; a < pack->count; a++)
    supla_client_channelgroup_update(scd, &pack->items[a], 0);

  srpc_cs_async_get_next(scd->srpc);
}

void supla_client_channel_relation_update(
    TSuplaClientData *scd, TSC_SuplaChannelRelation *channel_relation) {
  if (scd->cfg.cb_channel_relation_update)
    scd->cfg.cb_channel_relation_update(scd, scd->cfg.user_data,
                                        channel_relation);
}

void supla_client_channel_relation_pack_update(
    TSuplaClientData *scd, TSC_SuplaChannelRelationPack *pack) {
  int a;

  for (a = 0; a < pack->count; a++)
    supla_client_channel_relation_update(scd, &pack->items[a]);

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

void supla_client_scene_pack_update(TSuplaClientData *scd,
                                    TSC_SuplaScenePack *pack) {
  int a;

  if (!scd->cfg.cb_scene_update) {
    return;
  }

  for (a = 0; a < pack->count; a++) {
    TSC_SuplaScene scene = {};

    scene.EOL = pack->items[a].EOL;
    scene.Id = pack->items[a].Id;
    scene.LocationId = pack->items[a].LocationId;
    scene.AltIcon = pack->items[a].AltIcon;
    scene.UserIcon = pack->items[a].UserIcon;

    scene.CaptionSize = pack->items[a].CaptionSize;
    memcpy(scene.Caption, pack->items[a].Caption, SUPLA_SCENE_CAPTION_MAXSIZE);

    supla_client__set_str(scene.Caption, &scene.CaptionSize,
                          SUPLA_SCENE_CAPTION_MAXSIZE);
    scd->cfg.cb_scene_update(scd, scd->cfg.user_data, &scene);
  }

  srpc_cs_async_get_next(scd->srpc);
}

void supla_client_scene_state_pack_update(TSuplaClientData *scd,
                                          TSC_SuplaSceneStatePack *pack) {
  int a;

  if (!scd->cfg.cb_scene_state_update) {
    return;
  }

  for (a = 0; a < pack->count; a++) {
    TSC_SuplaSceneState state = {};

    state.EOL = pack->items[a].EOL;
    state.SceneId = pack->items[a].SceneId;
    state.MillisecondsFromStart = pack->items[a].MillisecondsFromStart;
    state.MillisecondsLeft = pack->items[a].MillisecondsLeft;
    state.InitiatorId = pack->items[a].InitiatorId;

    state.InitiatorNameSize = pack->items[a].InitiatorNameSize;
    memcpy(state.InitiatorName, pack->items[a].InitiatorName,
           SUPLA_INITIATOR_NAME_MAXSIZE);

    supla_client__set_str(state.InitiatorName, &state.InitiatorNameSize,
                          SUPLA_INITIATOR_NAME_MAXSIZE);
    scd->cfg.cb_scene_state_update(scd, scd->cfg.user_data, &state);
  }

  srpc_cs_async_get_next(scd->srpc);
}

void supla_client_channel_value_update_b(TSuplaClientData *scd,
                                         TSC_SuplaChannelValue_B *channel_value,
                                         char gn) {
  if (scd->cfg.cb_channel_value_update)
    scd->cfg.cb_channel_value_update(scd, scd->cfg.user_data, channel_value);

  if (gn == 1) {
    srpc_cs_async_get_next(scd->srpc);
  }
}

void supla_client_channel_value_update(TSuplaClientData *scd,
                                       TSC_SuplaChannelValue *channel_value,
                                       char gn) {
  TSC_SuplaChannelValue_B value = {};

  value.EOL = channel_value->EOL;
  value.Id = channel_value->Id;
  value.online = channel_value->online;
  memcpy(value.value.value, channel_value->value.value,
         SUPLA_CHANNELVALUE_SIZE);
  memcpy(value.value.sub_value, channel_value->value.sub_value,
         SUPLA_CHANNELVALUE_SIZE);

  supla_client_channel_value_update_b(scd, &value, gn);
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

void supla_client_channelvalue_pack_update_b(
    TSuplaClientData *scd, TSC_SuplaChannelValuePack_B *pack) {
  int a;

  if (pack && pack->count <= SUPLA_CHANNELVALUE_PACK_MAXCOUNT) {
    for (a = 0; a < pack->count; a++) {
      supla_client_channel_value_update_b(scd, &pack->items[a], 0);
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

void supla_client_channel_b2c(TSC_SuplaChannel_B *b, TSC_SuplaChannel_C *c) {
  c->EOL = b->EOL;
  c->Id = b->Id;
  c->DeviceID = 0;
  c->LocationID = b->LocationID;
  c->Func = b->Func;
  c->AltIcon = b->AltIcon;
  c->Type = 0;
  c->Flags = b->Flags;
  c->UserIcon = 0;
  c->ManufacturerID = 0;
  c->ProductID = 0;
  c->ProtocolVersion = 0;
  c->online = b->online;
  memcpy(&c->value, &b->value, sizeof(TSuplaChannelValue));
  c->CaptionSize = b->CaptionSize;
  memcpy(c->Caption, b->Caption, SUPLA_CHANNEL_CAPTION_MAXSIZE);
}

void supla_client_channel_c2d(TSC_SuplaChannel_C *c, TSC_SuplaChannel_D *d) {
  d->EOL = c->EOL;
  d->Id = c->Id;
  d->DeviceID = c->DeviceID;
  d->LocationID = c->LocationID;
  d->Func = c->Func;
  d->AltIcon = c->AltIcon;
  d->Type = c->Type;
  d->Flags = c->Flags;
  d->UserIcon = c->UserIcon;
  d->ManufacturerID = c->ManufacturerID;
  d->ProductID = c->ProductID;
  d->ProtocolVersion = c->ProtocolVersion;
  d->online = c->online;
  d->CaptionSize = c->CaptionSize;
  memcpy(d->Caption, c->Caption, SUPLA_CHANNEL_CAPTION_MAXSIZE);

  d->value.sub_value_type = SUBV_TYPE_NOT_SET_OR_OFFLINE;
  memcpy(d->value.value, c->value.value, SUPLA_CHANNELVALUE_SIZE);
  memcpy(d->value.sub_value, c->value.sub_value, SUPLA_CHANNELVALUE_SIZE);
}

void supla_client_channel_d2e(TSC_SuplaChannel_D *d, TSC_SuplaChannel_E *e) {
  e->EOL = d->EOL;
  e->Id = d->Id;
  e->DeviceID = d->DeviceID;
  e->LocationID = d->LocationID;
  e->Func = d->Func;
  e->AltIcon = d->AltIcon;
  e->Type = d->Type;
  e->Flags = d->Flags;
  e->UserIcon = d->UserIcon;
  e->ManufacturerID = d->ManufacturerID;
  e->ProductID = d->ProductID;
  e->ProtocolVersion = d->ProtocolVersion;
  e->online = d->online;
  e->CaptionSize = d->CaptionSize;
  e->DefaultConfigCRC32 = 0;
  memcpy(e->Caption, d->Caption, SUPLA_CHANNEL_CAPTION_MAXSIZE);

  e->value.sub_value_type = d->value.sub_value_type;
  memcpy(e->value.value, d->value.value, SUPLA_CHANNELVALUE_SIZE);
  memcpy(e->value.sub_value, d->value.sub_value, SUPLA_CHANNELVALUE_SIZE);
}

void supla_client_channel_update_d(TSuplaClientData *scd,
                                   TSC_SuplaChannel_D *channel_d, char gn) {
  TSC_SuplaChannel_E channel_e = {};

  supla_client_channel_d2e(channel_d, &channel_e);
  supla_client_channel_update_e(scd, &channel_e, gn);
}

void supla_client_channel_update_c(TSuplaClientData *scd,
                                   TSC_SuplaChannel_C *channel_c, char gn) {
  TSC_SuplaChannel_D channel_d = {};

  supla_client_channel_c2d(channel_c, &channel_d);
  supla_client_channel_update_d(scd, &channel_d, gn);
}

void supla_client_channel_update_b(TSuplaClientData *scd,
                                   TSC_SuplaChannel_B *channel_b, char gn) {
  TSC_SuplaChannel_C channel_c;
  memset(&channel_c, 0, sizeof(TSC_SuplaChannel_C));

  supla_client_channel_b2c(channel_b, &channel_c);
  supla_client_channel_update_c(scd, &channel_c, gn);
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

void supla_client_channelpack_update_c(TSuplaClientData *scd,
                                       TSC_SuplaChannelPack_C *pack) {
  int a;

  for (a = 0; a < pack->count; a++)
    supla_client_channel_update_c(scd, &pack->items[a], 0);

  srpc_cs_async_get_next(scd->srpc);
}

void supla_client_channelpack_update_d(TSuplaClientData *scd,
                                       TSC_SuplaChannelPack_D *pack) {
  int a;

  for (a = 0; a < pack->count; a++)
    supla_client_channel_update_d(scd, &pack->items[a], 0);

  srpc_cs_async_get_next(scd->srpc);
}

void supla_client_channelpack_update_e(TSuplaClientData *scd,
                                       TSC_SuplaChannelPack_E *pack) {
  int a;

  for (a = 0; a < pack->count; a++)
    supla_client_channel_update_e(scd, &pack->items[a], 0);

  srpc_cs_async_get_next(scd->srpc);
}

void supla_client_on_event(TSuplaClientData *scd, TSC_SuplaEvent *event) {
  supla_client_set_str(event->SenderName, &event->SenderNameSize,
                       SUPLA_SENDER_NAME_MAXSIZE);

  if (scd->cfg.cb_on_event)
    scd->cfg.cb_on_event(scd, scd->cfg.user_data, event);
}

void supla_client_on_oauth_token_request_result(
    TSuplaClientData *scd, TSC_OAuthTokenRequestResult *result) {
  supla_client_set_str(result->Token.Token, &result->Token.TokenSize,
                       SUPLA_OAUTH_TOKEN_MAXSIZE);

  if (scd->cfg.cb_on_oauth_token_request_result)
    scd->cfg.cb_on_oauth_token_request_result(scd, scd->cfg.user_data, result);
}

TCalCfg_ZWave_Node *supla_client_zwave_node(TSC_DeviceCalCfgResult *result) {
  TCalCfg_ZWave_Node *node = NULL;
  if (result != NULL && result->DataSize <= sizeof(TCalCfg_ZWave_Node) &&
      result->DataSize >=
          sizeof(TCalCfg_ZWave_Node) - ZWAVE_NODE_NAME_MAXSIZE &&
      result->DataSize <= SUPLA_CALCFG_DATA_MAXSIZE) {
    node = (TCalCfg_ZWave_Node *)result->Data;
    unsigned int NameSize = node->NameSize;
    supla_client_set_str(node->Name, &NameSize, ZWAVE_NODE_NAME_MAXSIZE);
    node->NameSize = NameSize;
  }
  return node;
}

void supla_client_on_device_calcfg_result(TSuplaClientData *scd,
                                          TSC_DeviceCalCfgResult *result) {
  if (scd->cfg.cb_on_device_calcfg_result) {
    scd->cfg.cb_on_device_calcfg_result(scd, scd->cfg.user_data, result);
  }

  switch (result->Command) {
    case SUPLA_CALCFG_CMD_PROGRESS_REPORT:
      if (result->Result == SUPLA_CALCFG_RESULT_TRUE &&
          result->DataSize == sizeof(TCalCfg_ProgressReport) &&
          scd->cfg.cb_on_device_calcfg_progress_report) {
        scd->cfg.cb_on_device_calcfg_progress_report(
            scd, scd->cfg.user_data, result->ChannelID,
            (TCalCfg_ProgressReport *)result->Data);
      }
      break;
    case SUPLA_CALCFG_CMD_DEBUG_STRING:
      if (result->DataSize > 0 && scd->cfg.cb_on_device_calcfg_debug_string) {
        if (result->DataSize >= SUPLA_CALCFG_DATA_MAXSIZE) {
          result->DataSize = SUPLA_CALCFG_DATA_MAXSIZE - 1;
          result->Data[result->DataSize] = 0;
        }

        scd->cfg.cb_on_device_calcfg_debug_string(scd, scd->cfg.user_data,
                                                  result->Data);
      }
      break;
    case SUPLA_CALCFG_CMD_ZWAVE_RESET_AND_CLEAR:
      if (scd->cfg.cb_on_zwave_reset_and_clear_result) {
        scd->cfg.cb_on_zwave_reset_and_clear_result(scd, scd->cfg.user_data,
                                                    result->Result);
      }
      break;
    case SUPLA_CALCFG_CMD_ZWAVE_ADD_NODE:
      if (scd->cfg.cb_on_zwave_add_node_result) {
        scd->cfg.cb_on_zwave_add_node_result(scd, scd->cfg.user_data,
                                             result->Result,
                                             supla_client_zwave_node(result));
      }
      break;
    case SUPLA_CALCFG_CMD_ZWAVE_REMOVE_NODE:
      if (scd->cfg.cb_on_zwave_remove_node_result) {
        scd->cfg.cb_on_zwave_remove_node_result(
            scd, scd->cfg.user_data, result->Result,
            result->DataSize == sizeof(unsigned char)
                ? (unsigned char)result->Data[0]
                : 0);
      }
      break;
    case SUPLA_CALCFG_CMD_ZWAVE_GET_NODE_LIST:
      if (scd->cfg.cb_on_zwave_get_node_list_result) {
        scd->cfg.cb_on_zwave_get_node_list_result(
            scd, scd->cfg.user_data, result->Result,
            supla_client_zwave_node(result));
      }
      break;
    case SUPLA_CALCFG_CMD_ZWAVE_GET_ASSIGNED_NODE_ID:
      if (scd->cfg.cb_on_zwave_get_assigned_node_id_result) {
        scd->cfg.cb_on_zwave_get_assigned_node_id_result(
            scd, scd->cfg.user_data, result->Result,
            result->DataSize == sizeof(unsigned char)
                ? (unsigned char)result->Data[0]
                : 0);
      }
      break;
    case SUPLA_CALCFG_CMD_ZWAVE_GET_WAKE_UP_SETTINGS:
      if (scd->cfg.cb_on_zwave_wake_up_settings_report) {
        scd->cfg.cb_on_zwave_wake_up_settings_report(
            scd, scd->cfg.user_data, result->Result,
            result->DataSize == sizeof(TCalCfg_ZWave_WakeupSettingsReport)
                ? (TCalCfg_ZWave_WakeupSettingsReport *)result->Data
                : NULL);
      }
      break;

    case SUPLA_CALCFG_CMD_ZWAVE_SET_WAKE_UP_TIME:
      if (scd->cfg.cb_on_zwave_set_wake_up_time_result) {
        scd->cfg.cb_on_zwave_set_wake_up_time_result(scd, scd->cfg.user_data,
                                                     result->Result);
      }
      break;

    case SUPLA_CALCFG_CMD_ZWAVE_ASSIGN_NODE_ID:
      if (scd->cfg.cb_on_zwave_assign_node_id_result) {
        scd->cfg.cb_on_zwave_assign_node_id_result(
            scd, scd->cfg.user_data, result->Result,
            result->DataSize == sizeof(unsigned char)
                ? (unsigned char)result->Data[0]
                : 0);
      }
      break;
  }
}

void supla_client_on_remote_call_received(void *_srpc, unsigned int rr_id,
                                          unsigned int call_id, void *_scd,
                                          unsigned char proto_version) {
  TsrpcReceivedData rd;
  char result;
  TSuplaClientData *scd = (TSuplaClientData *)_scd;

  gettimeofday(&scd->last_call_recv, NULL);

  supla_log(LOG_DEBUG, "on_remote_call_received: %i", call_id);

  if (SUPLA_RESULT_TRUE == (result = srpc_getdata(_srpc, &rd, 0))) {
    switch (rd.call_id) {
      case SUPLA_SDC_CALL_GETVERSION_RESULT:
        if (scd->cfg.cb_on_getversion_result && rd.data.sdc_getversion_result) {
          rd.data.sdc_getversion_result->SoftVer[SUPLA_SOFTVER_MAXSIZE - 1] = 0;
          scd->cfg.cb_on_getversion_result(scd, scd->cfg.user_data,
                                           rd.data.sdc_getversion_result);
        }
        break;
      case SUPLA_SDC_CALL_VERSIONERROR:
        if (rd.data.sdc_version_error) {
          supla_client_on_version_error(scd, rd.data.sdc_version_error);
        }
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

        if (rd.data.sc_register_client_result_b == NULL) {
          break;
        }

        {
          TSC_SuplaRegisterClientResult_C *sc_register_client_result_c =
              (TSC_SuplaRegisterClientResult_C *)malloc(
                  sizeof(TSC_SuplaRegisterClientResult_C));

          if (sc_register_client_result_c == NULL) {
            break;
          } else {
            sc_register_client_result_c->result_code =
                rd.data.sc_register_client_result_b->result_code;

            sc_register_client_result_c->ClientID =
                rd.data.sc_register_client_result_b->ClientID;

            sc_register_client_result_c->LocationCount =
                rd.data.sc_register_client_result_b->LocationCount;

            sc_register_client_result_c->ChannelCount =
                rd.data.sc_register_client_result_b->ChannelCount;

            sc_register_client_result_c->ChannelGroupCount =
                rd.data.sc_register_client_result_b->ChannelGroupCount;
            sc_register_client_result_c->Flags =
                rd.data.sc_register_client_result_b->Flags;

            sc_register_client_result_c->activity_timeout =
                rd.data.sc_register_client_result_b->activity_timeout;

            sc_register_client_result_c->version =
                rd.data.sc_register_client_result_b->version;

            sc_register_client_result_c->version_min =
                rd.data.sc_register_client_result_b->version_min;

            sc_register_client_result_c->serverUnixTimestamp = 0;

            free(rd.data.sc_register_client_result_b);
            rd.data.sc_register_client_result_c = sc_register_client_result_c;
          }
        }

      /* no break between SUPLA_SC_CALL_REGISTER_CLIENT_RESULT_B and
       * SUPLA_SC_CALL_REGISTER_CLIENT_RESULT_C!!! */
      case SUPLA_SC_CALL_REGISTER_CLIENT_RESULT_C:
        if (rd.data.sc_register_client_result_c == NULL) {
          break;
        }

        {
          TSC_SuplaRegisterClientResult_D *sc_register_client_result_d =
              (TSC_SuplaRegisterClientResult_D *)malloc(
                  sizeof(TSC_SuplaRegisterClientResult_D));

          if (sc_register_client_result_d == NULL) {
            break;
          } else {
            sc_register_client_result_d->result_code =
                rd.data.sc_register_client_result_c->result_code;

            sc_register_client_result_d->ClientID =
                rd.data.sc_register_client_result_c->ClientID;

            sc_register_client_result_d->LocationCount =
                rd.data.sc_register_client_result_c->LocationCount;

            sc_register_client_result_d->ChannelCount =
                rd.data.sc_register_client_result_c->ChannelCount;

            sc_register_client_result_d->ChannelGroupCount =
                rd.data.sc_register_client_result_c->ChannelGroupCount;
            sc_register_client_result_d->Flags =
                rd.data.sc_register_client_result_c->Flags;

            sc_register_client_result_d->activity_timeout =
                rd.data.sc_register_client_result_c->activity_timeout;

            sc_register_client_result_d->version =
                rd.data.sc_register_client_result_c->version;

            sc_register_client_result_d->version_min =
                rd.data.sc_register_client_result_c->version_min;

            sc_register_client_result_d->serverUnixTimestamp =
                rd.data.sc_register_client_result_c->serverUnixTimestamp;

            sc_register_client_result_d->SceneCount = 0;

            free(rd.data.sc_register_client_result_c);
            rd.data.sc_register_client_result_d = sc_register_client_result_d;
          }
        }

        /* no break between SUPLA_SC_CALL_REGISTER_CLIENT_RESULT_C and
         * SUPLA_SC_CALL_REGISTER_CLIENT_RESULT_D!!! */
      case SUPLA_SC_CALL_REGISTER_CLIENT_RESULT_D:
        if (rd.data.sc_register_client_result_d) {
          scd->server_time_diff = 0;
          if (rd.data.sc_register_client_result_d->serverUnixTimestamp) {
            struct timeval now;
            gettimeofday(&now, NULL);
            scd->server_time_diff =
                now.tv_sec -
                rd.data.sc_register_client_result_d->serverUnixTimestamp;
          }
          supla_client_on_register_result(scd,
                                          rd.data.sc_register_client_result_d);
        }
        break;
      case SUPLA_SC_CALL_LOCATION_UPDATE:
        if (rd.data.sc_location) {
          supla_client_location_update(scd, rd.data.sc_location, 1);
        }
        break;
      case SUPLA_SC_CALL_LOCATIONPACK_UPDATE:
        if (rd.data.sc_location_pack) {
          supla_client_locationpack_update(scd, rd.data.sc_location_pack);
        }
        break;
      case SUPLA_SC_CALL_CHANNELPACK_UPDATE:
        if (rd.data.sc_channel_pack) {
          supla_client_channelpack_update(scd, rd.data.sc_channel_pack);
        }
        break;
      case SUPLA_SC_CALL_CHANNELPACK_UPDATE_B:
        if (rd.data.sc_channel_pack_b) {
          supla_client_channelpack_update_b(scd, rd.data.sc_channel_pack_b);
        }
        break;
      case SUPLA_SC_CALL_CHANNELPACK_UPDATE_C:
        if (rd.data.sc_channel_pack_c) {
          supla_client_channelpack_update_c(scd, rd.data.sc_channel_pack_c);
        }
        break;
      case SUPLA_SC_CALL_CHANNELPACK_UPDATE_D:
        if (rd.data.sc_channel_pack_d) {
          supla_client_channelpack_update_d(scd, rd.data.sc_channel_pack_d);
        }
        break;
      case SUPLA_SC_CALL_CHANNELPACK_UPDATE_E:
        if (rd.data.sc_channel_pack_e) {
          supla_client_channelpack_update_e(scd, rd.data.sc_channel_pack_e);
        }
        break;
      case SUPLA_SC_CALL_CHANNEL_VALUE_UPDATE:
        if (rd.data.sc_channel_value) {
          supla_client_channel_value_update(scd, rd.data.sc_channel_value, 1);
        }
        break;
      case SUPLA_SC_CALL_CHANNEL_RELATION_PACK_UPDATE:
        if (rd.data.sc_channel_relation_pack) {
          supla_client_channel_relation_pack_update(
              scd, rd.data.sc_channel_relation_pack);
        }
        break;
      case SUPLA_SC_CALL_CHANNELGROUP_PACK_UPDATE:
        if (rd.data.sc_channelgroup_pack) {
          supla_client_channelgroup_pack_update(scd,
                                                rd.data.sc_channelgroup_pack);
        }
        break;
      case SUPLA_SC_CALL_CHANNELGROUP_PACK_UPDATE_B:
        if (rd.data.sc_channelgroup_pack_b) {
          supla_client_channelgroup_pack_update_b(
              scd, rd.data.sc_channelgroup_pack_b);
        }
        break;
      case SUPLA_SC_CALL_CHANNELGROUP_RELATION_PACK_UPDATE:
        if (rd.data.sc_channelgroup_relation_pack) {
          supla_client_channelgroup_relation_pack_update(
              scd, rd.data.sc_channelgroup_relation_pack);
        }
        break;
      case SUPLA_SC_CALL_SCENE_PACK_UPDATE:
        if (rd.data.sc_scene_pack) {
          supla_client_scene_pack_update(scd, rd.data.sc_scene_pack);
        }
        break;
      case SUPLA_SC_CALL_SCENE_STATE_PACK_UPDATE:
        if (rd.data.sc_scene_state_pack) {
          supla_client_scene_state_pack_update(scd,
                                               rd.data.sc_scene_state_pack);
        }
        break;
      case SUPLA_SC_CALL_CHANNELVALUE_PACK_UPDATE:
        if (rd.data.sc_channelvalue_pack) {
          supla_client_channelvalue_pack_update(scd,
                                                rd.data.sc_channelvalue_pack);
        }
        break;
      case SUPLA_SC_CALL_CHANNELVALUE_PACK_UPDATE_B:
        if (rd.data.sc_channelvalue_pack_b) {
          supla_client_channelvalue_pack_update_b(
              scd, rd.data.sc_channelvalue_pack_b);
        }
        break;
      case SUPLA_SC_CALL_CHANNELEXTENDEDVALUE_PACK_UPDATE:
        if (rd.data.sc_channelextendedvalue_pack) {
          supla_client_channelextendedvalue_pack_update(
              scd, rd.data.sc_channelextendedvalue_pack);
        }
        break;
      case SUPLA_SC_CALL_EVENT:
        if (rd.data.sc_event) {
          supla_client_on_event(scd, rd.data.sc_event);
        }
        break;
      case SUPLA_SDC_CALL_GET_REGISTRATION_ENABLED_RESULT:

        if (scd->cfg.cb_on_registration_enabled && rd.data.sdc_reg_enabled) {
          scd->cfg.cb_on_registration_enabled(scd, scd->cfg.user_data,
                                              rd.data.sdc_reg_enabled);
        }
        break;
      case SUPLA_SC_CALL_OAUTH_TOKEN_REQUEST_RESULT:
        if (scd->cfg.cb_on_oauth_token_request_result &&
            rd.data.sc_oauth_tokenrequest_result) {
          supla_client_on_oauth_token_request_result(
              scd, rd.data.sc_oauth_tokenrequest_result);
        }
        break;
      case SUPLA_SC_CALL_SUPERUSER_AUTHORIZATION_RESULT:
        if (scd->cfg.cb_on_superuser_authorization_result &&
            rd.data.sc_superuser_authorization_result) {
          scd->cfg.cb_on_superuser_authorization_result(
              scd, scd->cfg.user_data,
              rd.data.sc_superuser_authorization_result->Result ==
                  SUPLA_RESULTCODE_AUTHORIZED,
              rd.data.sc_superuser_authorization_result->Result);
        }
        break;
      case SUPLA_SC_CALL_DEVICE_CALCFG_RESULT:
        if (rd.data.sc_device_calcfg_result) {
          supla_client_on_device_calcfg_result(scd,
                                               rd.data.sc_device_calcfg_result);
        }
        break;
      case SUPLA_DSC_CALL_CHANNEL_STATE_RESULT:
        if (scd->cfg.cb_on_device_channel_state && rd.data.dsc_channel_state) {
          scd->cfg.cb_on_device_channel_state(scd, scd->cfg.user_data,
                                              rd.data.dsc_channel_state);
        }
        break;
      case SUPLA_SC_CALL_CHANNEL_BASIC_CFG_RESULT:
        if (scd->cfg.cb_on_channel_basic_cfg && rd.data.sc_channel_basic_cfg) {
          supla_client_set_str(rd.data.sc_channel_basic_cfg->Caption,
                               &rd.data.sc_channel_basic_cfg->CaptionSize,
                               SUPLA_CHANNEL_CAPTION_MAXSIZE);

          rd.data.sc_channel_basic_cfg
              ->DeviceName[SUPLA_DEVICE_NAME_MAXSIZE - 1] = 0;
          rd.data.sc_channel_basic_cfg
              ->DeviceSoftVer[SUPLA_SOFTVER_MAXSIZE - 1] = 0;

          scd->cfg.cb_on_channel_basic_cfg(scd, scd->cfg.user_data,
                                           rd.data.sc_channel_basic_cfg);
        }
        break;
      case SUPLA_SC_CALL_SET_CHANNEL_FUNCTION_RESULT:
        if (scd->cfg.cb_on_channel_function_set_result &&
            rd.data.sc_set_channel_function_result) {
          scd->cfg.cb_on_channel_function_set_result(
              scd, scd->cfg.user_data, rd.data.sc_set_channel_function_result);
        }
        break;
      case SUPLA_SCD_CALL_SET_CHANNEL_CAPTION_RESULT:
      case SUPLA_SC_CALL_SET_CHANNEL_GROUP_CAPTION_RESULT:
      case SUPLA_SC_CALL_SET_LOCATION_CAPTION_RESULT:
      case SUPLA_SC_CALL_SET_SCENE_CAPTION_RESULT:
        if (rd.data.scd_set_caption_result) {
          supla_client_set_str(rd.data.scd_set_caption_result->Caption,
                               &rd.data.scd_set_caption_result->CaptionSize,
                               SUPLA_CAPTION_MAXSIZE);

          if (rd.call_id == SUPLA_SCD_CALL_SET_CHANNEL_CAPTION_RESULT &&
              scd->cfg.cb_on_channel_caption_set_result) {
            scd->cfg.cb_on_channel_caption_set_result(
                scd, scd->cfg.user_data, rd.data.scd_set_caption_result);
          } else if (rd.call_id ==
                         SUPLA_SC_CALL_SET_CHANNEL_GROUP_CAPTION_RESULT &&
                     scd->cfg.cb_on_channel_group_caption_set_result) {
            scd->cfg.cb_on_channel_group_caption_set_result(
                scd, scd->cfg.user_data, rd.data.scd_set_caption_result);
          } else if (rd.call_id == SUPLA_SC_CALL_SET_LOCATION_CAPTION_RESULT &&
                     scd->cfg.cb_on_location_caption_set_result) {
            scd->cfg.cb_on_location_caption_set_result(
                scd, scd->cfg.user_data, rd.data.scd_set_caption_result);
          } else if (rd.call_id == SUPLA_SC_CALL_SET_SCENE_CAPTION_RESULT &&
                     scd->cfg.cb_on_scene_caption_set_result) {
            scd->cfg.cb_on_scene_caption_set_result(
                scd, scd->cfg.user_data, rd.data.scd_set_caption_result);
          }
        }
        break;
      case SUPLA_SC_CALL_CLIENTS_RECONNECT_REQUEST_RESULT:
        if (scd->cfg.cb_on_clients_reconnect_request_result &&
            rd.data.sc_clients_reconnect_result) {
          scd->cfg.cb_on_clients_reconnect_request_result(
              scd, scd->cfg.user_data, rd.data.sc_clients_reconnect_result);
        }
        break;
      case SUPLA_SC_CALL_SET_REGISTRATION_ENABLED_RESULT:
        if (scd->cfg.cb_on_set_registration_enabled_result &&
            rd.data.sc_set_registration_enabled_result) {
          scd->cfg.cb_on_set_registration_enabled_result(
              scd, scd->cfg.user_data,
              rd.data.sc_set_registration_enabled_result);
        }
        break;
      case SUPLA_SC_CALL_ACTION_EXECUTION_RESULT:
        if (scd->cfg.cb_on_action_execution_result) {
          scd->cfg.cb_on_action_execution_result(
              scd, scd->cfg.user_data, rd.data.sc_action_execution_result);
        }
        break;
      case SUPLA_SC_CALL_GET_CHANNEL_VALUE_RESULT:
        if (scd->cfg.cb_on_get_channel_value_result) {
          scd->cfg.cb_on_get_channel_value_result(scd, scd->cfg.user_data,
                                                  rd.data.sc_get_value_result);
        }
        break;
      case SUPLA_SC_CALL_CHANNEL_CONFIG_UPDATE_OR_RESULT:
        if (scd->cfg.cb_on_channel_config_update_or_result) {
          unsigned _supla_int_t crc32 = 0;
          if (srpc_get_proto_version(_srpc) >= 23) {
            crc32 = st_crc32_checksum(
                (unsigned char *)
                    rd.data.sc_channel_config_update_or_result->Config.Config,
                rd.data.sc_channel_config_update_or_result->Config.ConfigSize);
          }
          scd->cfg.cb_on_channel_config_update_or_result(
              scd, scd->cfg.user_data,
              rd.data.sc_channel_config_update_or_result, crc32);
        }
        break;
      case SUPLA_SC_CALL_DEVICE_CONFIG_UPDATE_OR_RESULT:
        if (scd->cfg.cb_on_device_config_update_or_result) {
          scd->cfg.cb_on_device_config_update_or_result(
              scd, scd->cfg.user_data,
              rd.data.sc_device_config_update_or_result);
        }
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

char supla_client_connect(void *_suplaclient, int conn_timeout_ms) {
  TSuplaClientData *suplaclient = (TSuplaClientData *)_suplaclient;
  supla_client_disconnect(_suplaclient);

  supla_client_clean(_suplaclient);

  int err = 0;

  if (ssocket_client_connect(suplaclient->ssd, NULL, &err, conn_timeout_ms) ==
      1) {
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
      srpc_call_allowed(suplaclient->srpc, SUPLA_CS_CALL_REGISTER_CLIENT_D)) {
    TCS_SuplaRegisterClient_D src;
    memset(&src, 0, sizeof(TCS_SuplaRegisterClient_D));

#ifdef _WIN32
    _snprintf_s(src.Email, SUPLA_EMAIL_MAXSIZE, _TRUNCATE, "%s",
                suplaclient->cfg.Email);
    _snprintf_s(src.Password, SUPLA_PASSWORD_MAXSIZE, _TRUNCATE, "%s",
                suplaclient->cfg.Password);
    _snprintf_s(src.Name, SUPLA_CLIENT_NAME_MAXSIZE, _TRUNCATE, "%s",
                suplaclient->cfg.Name);
    _snprintf_s(src.SoftVer, SUPLA_SOFTVER_MAXSIZE, _TRUNCATE, "%s",
                suplaclient->cfg.SoftVer);
    _snprintf_s(src.ServerName, SUPLA_SERVER_NAME_MAXSIZE, _TRUNCATE, "%s",
                suplaclient->cfg.host);
#else
    snprintf(src.Email, SUPLA_EMAIL_MAXSIZE, "%s", suplaclient->cfg.Email);
    snprintf(src.Password, SUPLA_PASSWORD_MAXSIZE, "%s",
             suplaclient->cfg.Password);
    snprintf(src.Name, SUPLA_CLIENT_NAME_MAXSIZE, "%s", suplaclient->cfg.Name);
    snprintf(src.SoftVer, SUPLA_SOFTVER_MAXSIZE, "%s",
             suplaclient->cfg.SoftVer);
    snprintf(src.ServerName, SUPLA_SERVER_NAME_MAXSIZE, "%s",
             suplaclient->cfg.host);
#endif

    memcpy(src.AuthKey, suplaclient->cfg.AuthKey, SUPLA_AUTHKEY_SIZE);
    memcpy(src.GUID, suplaclient->cfg.clientGUID, SUPLA_GUID_SIZE);
    srpc_cs_async_registerclient_d(suplaclient->srpc, &src);

  } else if (strnlen(suplaclient->cfg.Email, SUPLA_EMAIL_MAXSIZE) > 0 &&
             srpc_call_allowed(suplaclient->srpc,
                               SUPLA_CS_CALL_REGISTER_CLIENT_C)) {
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

char supla_client__iterate(void *_suplaclient, unsigned char reg,
                           int wait_usec) {
  TSuplaClientData *suplaclient = (TSuplaClientData *)_suplaclient;

  if (supla_client_connected(_suplaclient) == 0) return 0;

  if (reg && supla_client_registered(_suplaclient) == 0) {
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

char supla_client_iterate(void *_suplaclient, int wait_usec) {
  return supla_client__iterate(_suplaclient, 1, wait_usec);
}

void supla_client_raise_event(void *_suplaclient) {
  eh_raise_event(((TSuplaClientData *)_suplaclient)->eh);
}

void *supla_client_get_userdata(void *_suplaclient) {
  TSuplaClientData *suplaclient = (TSuplaClientData *)_suplaclient;
  return suplaclient->cfg.user_data;
}

char supla_client_send_raw_value(void *_suplaclient, int ID,
                                 char value[SUPLA_CHANNELVALUE_SIZE],
                                 char Target) {
  TSuplaClientData *suplaclient = (TSuplaClientData *)_suplaclient;
  char result = 0;

  lck_lock(suplaclient->lck);
  if (supla_client_registered(_suplaclient) == 1) {
    if (srpc_get_proto_version(suplaclient->srpc) >= 9) {
      TCS_SuplaNewValue _value;
      memset(&_value, 0, sizeof(TCS_SuplaNewValue));
      _value.Id = ID;
      _value.Target = Target;
      memcpy(_value.value, value, SUPLA_CHANNELVALUE_SIZE);
      result = srpc_cs_async_set_value(suplaclient->srpc, &_value) ==
                       SUPLA_RESULT_FALSE
                   ? 0
                   : 1;
    } else {
      TCS_SuplaChannelNewValue_B _value;
      memset(&_value, 0, sizeof(TCS_SuplaChannelNewValue_B));
      _value.ChannelId = ID;
      memcpy(_value.value, value, SUPLA_CHANNELVALUE_SIZE);
      result = srpc_cs_async_set_channel_value_b(suplaclient->srpc, &_value) ==
                       SUPLA_RESULT_FALSE
                   ? 0
                   : 1;
    }
  }
  lck_unlock(suplaclient->lck);

  return result;
}

char supla_client_open(void *_suplaclient, int ID, char group, char open) {
  char value[SUPLA_CHANNELVALUE_SIZE];
  memset(value, 0, SUPLA_CHANNELVALUE_SIZE);
  value[0] = open;

  return supla_client_send_raw_value(
      _suplaclient, ID, value,
      group > 0 ? SUPLA_TARGET_GROUP : SUPLA_TARGET_CHANNEL);
}

void _supla_client_set_rgbw_value(char *value, int color, char color_brightness,
                                  char brightness, char turn_onoff) {
  value[0] = brightness;
  value[1] = color_brightness;
  value[2] = (char)((color & 0x000000FF));        // BLUE
  value[3] = (char)((color & 0x0000FF00) >> 8);   // GREEN
  value[4] = (char)((color & 0x00FF0000) >> 16);  // RED
  value[5] = turn_onoff > 0 ? 1 : 0;
}

char supla_client_set_rgbw(void *_suplaclient, int ID, char group, int color,
                           char color_brightness, char brightness,
                           char turn_onoff) {
  TSuplaClientData *suplaclient = (TSuplaClientData *)_suplaclient;
  char result = 0;

  lck_lock(suplaclient->lck);
  if (supla_client_registered(_suplaclient) == 1) {
    if (srpc_get_proto_version(suplaclient->srpc) >= 9) {
      TCS_SuplaNewValue value;
      memset(&value, 0, sizeof(TCS_SuplaNewValue));
      _supla_client_set_rgbw_value(value.value, color, color_brightness,
                                   brightness, turn_onoff);
      value.Id = ID;
      value.Target = group > 0 ? SUPLA_TARGET_GROUP : SUPLA_TARGET_CHANNEL;
      result = srpc_cs_async_set_value(suplaclient->srpc, &value) ==
                       SUPLA_RESULT_FALSE
                   ? 0
                   : 1;
    } else {
      TCS_SuplaChannelNewValue_B value;
      memset(&value, 0, sizeof(TCS_SuplaChannelNewValue_B));
      _supla_client_set_rgbw_value(value.value, color, color_brightness,
                                   brightness, turn_onoff);
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
                             char brightness, char turn_onoff) {
  return supla_client_set_rgbw(_suplaclient, ID, group, 0, 0, brightness,
                               turn_onoff);
}

char supla_client_get_registration_enabled(void *_suplaclient) {
  return srpc_dcs_async_get_registration_enabled(
      ((TSuplaClientData *)_suplaclient)->srpc);
}

unsigned char supla_client_get_proto_version(void *_suplaclient) {
  return srpc_get_proto_version(((TSuplaClientData *)_suplaclient)->srpc);
}

char supla_client_get_version(void *_suplaclient) {
  return srpc_dcs_async_getversion(((TSuplaClientData *)_suplaclient)->srpc);
}

char supla_client_oauth_token_request(void *_suplaclient) {
  return srpc_cs_async_oauth_token_request(
             ((TSuplaClientData *)_suplaclient)->srpc) > 0;
}

char supla_client_superuser_authorization_request(void *_suplaclient,
                                                  const char *email,
                                                  const char *password) {
  TCS_SuperUserAuthorizationRequest request;
  snprintf(
      request.Email, SUPLA_EMAIL_MAXSIZE, "%s",
      email == NULL ? ((TSuplaClientData *)_suplaclient)->cfg.Email : email);
  snprintf(request.Password, SUPLA_PASSWORD_MAXSIZE, "%s", password);

  return srpc_cs_async_superuser_authorization_request(
      ((TSuplaClientData *)_suplaclient)->srpc, &request);
}

char supla_client_get_superuser_authorization_result(void *_suplaclient) {
  return srpc_cs_async_get_superuser_authorization_result(
      ((TSuplaClientData *)_suplaclient)->srpc);
}

char supla_client_device_calcfg_request(void *_suplaclient,
                                        TCS_DeviceCalCfgRequest_B *request) {
  if (request == NULL) return 0;
  if (srpc_get_proto_version(((TSuplaClientData *)_suplaclient)->srpc) >= 11) {
    return srpc_cs_async_device_calcfg_request_b(
        ((TSuplaClientData *)_suplaclient)->srpc, request);
  } else if (request->Target == SUPLA_TARGET_CHANNEL) {
    TCS_DeviceCalCfgRequest request_a;
    memset(&request_a, 0, sizeof(TCS_DeviceCalCfgRequest));

    request_a.ChannelID = request->Id;
    request_a.Command = request->Command;
    request_a.DataType = request->DataType;
    request_a.DataSize = request->DataSize;
    memcpy(request_a.Data, request->Data, SUPLA_CALCFG_DATA_MAXSIZE);

    return srpc_cs_async_device_calcfg_request(
        ((TSuplaClientData *)_suplaclient)->srpc, &request_a);
  }

  return 0;
}

char supla_client_device_calcfg_command(void *_suplaclient,
                                        _supla_int_t deviceId,
                                        _supla_int_t command) {
  TCS_DeviceCalCfgRequest_B request;
  memset(&request, 0, sizeof(TCS_DeviceCalCfgRequest_B));
  request.Target = SUPLA_TARGET_IODEVICE;
  request.Id = deviceId;
  request.Command = command;

  return supla_client_device_calcfg_request(_suplaclient, &request);
}

char supla_client_device_calcfg_cancel_all_commands(void *_suplaclient,
                                                    int DeviceID) {
  return supla_client_device_calcfg_command(
      _suplaclient, DeviceID, SUPLA_CALCFG_CMD_CANCEL_ALL_CLIENT_COMMANDS);
}

char supla_client_get_channel_state(void *_suplaclient, int ChannelID) {
  TCSD_ChannelStateRequest request;
  memset(&request, 0, sizeof(TCSD_ChannelStateRequest));

  request.ChannelID = ChannelID;
  return srpc_csd_async_get_channel_state(
      ((TSuplaClientData *)_suplaclient)->srpc, &request);
}

char supla_client_get_channel_basic_cfg(void *_suplaclient, int ChannelID) {
  return srpc_cs_async_get_channel_basic_cfg(
      ((TSuplaClientData *)_suplaclient)->srpc, ChannelID);
}

char supla_client_set_channel_function(void *_suplaclient, int ChannelID,
                                       int Function) {
  TCS_SetChannelFunction func;
  memset(&func, 0, sizeof(TCS_SetChannelFunction));
  func.ChannelID = ChannelID;
  func.Func = Function;

  return srpc_cs_async_set_channel_function(
      ((TSuplaClientData *)_suplaclient)->srpc, &func);
}

char supla_client_set_caption(void *_suplaclient, int ID, const char *Caption,
                              int CallID) {
  TDCS_SetCaption caption;
  memset(&caption, 0, sizeof(TDCS_SetCaption));

  caption.ID = ID;
  if (Caption != NULL) {
    caption.CaptionSize = strnlen(Caption, SUPLA_CAPTION_MAXSIZE) + 1;
    if (caption.CaptionSize > SUPLA_CAPTION_MAXSIZE) {
      caption.CaptionSize = SUPLA_CAPTION_MAXSIZE;
    }
    snprintf(caption.Caption, caption.CaptionSize, "%s", Caption);
  }

  switch (CallID) {
    case SUPLA_DCS_CALL_SET_CHANNEL_CAPTION:
      return srpc_dcs_async_set_channel_caption(
          ((TSuplaClientData *)_suplaclient)->srpc, &caption);
    case SUPLA_CS_CALL_SET_CHANNEL_GROUP_CAPTION:
      return srpc_cs_async_set_channel_group_caption(
          ((TSuplaClientData *)_suplaclient)->srpc, &caption);
    case SUPLA_CS_CALL_SET_LOCATION_CAPTION:
      return srpc_cs_async_set_location_caption(
          ((TSuplaClientData *)_suplaclient)->srpc, &caption);
    case SUPLA_CS_CALL_SET_SCENE_CAPTION:
      return srpc_cs_async_set_scene_caption(
          ((TSuplaClientData *)_suplaclient)->srpc, &caption);
  }

  return 0;
}

char supla_client_set_channel_caption(void *_suplaclient, int ChannelID,
                                      const char *Caption) {
  return supla_client_set_caption(_suplaclient, ChannelID, Caption,
                                  SUPLA_DCS_CALL_SET_CHANNEL_CAPTION);
}

char supla_client_set_channel_group_caption(void *_suplaclient,
                                            int ChannelGroupID,
                                            const char *Caption) {
  return supla_client_set_caption(_suplaclient, ChannelGroupID, Caption,
                                  SUPLA_CS_CALL_SET_CHANNEL_GROUP_CAPTION);
}

char supla_client_set_location_caption(void *_suplaclient, int LocationID,
                                       const char *Caption) {
  return supla_client_set_caption(_suplaclient, LocationID, Caption,
                                  SUPLA_CS_CALL_SET_LOCATION_CAPTION);
}

char supla_client_set_scene_caption(void *_suplaclient, int SceneID,
                                    const char *Caption) {
  return supla_client_set_caption(_suplaclient, SceneID, Caption,
                                  SUPLA_CS_CALL_SET_SCENE_CAPTION);
}

char supla_client_reconnect_all_clients(void *_suplaclient) {
  return srpc_cs_async_clients_reconnect_request(
      ((TSuplaClientData *)_suplaclient)->srpc);
}

char supla_client_set_registration_enabled(void *_suplaclient,
                                           int ioDeviceRegTimeSec,
                                           int clientRegTimeSec) {
  TCS_SetRegistrationEnabled re;
  memset(&re, 0, sizeof(TCS_SetRegistrationEnabled));
  re.IODeviceRegistrationTimeSec = ioDeviceRegTimeSec;
  re.ClientRegistrationTimeSec = clientRegTimeSec;

  return srpc_cs_async_set_registration_enabled(
      ((TSuplaClientData *)_suplaclient)->srpc, &re);
}

char supla_client_reconnect_device(void *_suplaclient, int deviceID) {
  TCS_DeviceReconnectRequest request;
  memset(&request, 0, sizeof(TCS_DeviceReconnectRequest));
  request.DeviceID = deviceID;

  return srpc_cs_async_device_reconnect_request(
      ((TSuplaClientData *)_suplaclient)->srpc, &request);
}

char supla_client_zwave_config_mode_active(void *_suplaclient, int deviceID) {
  return supla_client_device_calcfg_command(
      _suplaclient, deviceID, SUPLA_CALCFG_CMD_ZWAVE_CONFIG_MODE_ACTIVE);
}

char supla_client_zwave_reset_and_clear(void *_suplaclient, int deviceID) {
  return supla_client_device_calcfg_command(
      _suplaclient, deviceID, SUPLA_CALCFG_CMD_ZWAVE_RESET_AND_CLEAR);
}

char supla_client_zwave_add_node(void *_suplaclient, int deviceID) {
  return supla_client_device_calcfg_command(_suplaclient, deviceID,
                                            SUPLA_CALCFG_CMD_ZWAVE_ADD_NODE);
}

char supla_client_zwave_remove_node(void *_suplaclient, int deviceID) {
  return supla_client_device_calcfg_command(_suplaclient, deviceID,
                                            SUPLA_CALCFG_CMD_ZWAVE_REMOVE_NODE);
}

char supla_client_zwave_get_node_list(void *_suplaclient, int deviceID) {
  return supla_client_device_calcfg_command(
      _suplaclient, deviceID, SUPLA_CALCFG_CMD_ZWAVE_GET_NODE_LIST);
}

char supla_client_zwave_get_assigned_node_id(void *_suplaclient,
                                             int channelID) {
  TCS_DeviceCalCfgRequest_B request;
  memset(&request, 0, sizeof(TCS_DeviceCalCfgRequest_B));
  request.Target = SUPLA_TARGET_CHANNEL;
  request.Id = channelID;
  request.Command = SUPLA_CALCFG_CMD_ZWAVE_GET_ASSIGNED_NODE_ID;

  return supla_client_device_calcfg_request(_suplaclient, &request);
}

char supla_client_zwave_assign_node_id(void *_suplaclient, int channelID,
                                       unsigned char nodeID) {
  TCS_DeviceCalCfgRequest_B request;
  memset(&request, 0, sizeof(TCS_DeviceCalCfgRequest_B));
  request.Target = SUPLA_TARGET_CHANNEL;
  request.Id = channelID;
  request.Command = SUPLA_CALCFG_CMD_ZWAVE_ASSIGN_NODE_ID;

  request.DataSize = sizeof(unsigned char);
  memcpy(request.Data, &nodeID, request.DataSize);

  return supla_client_device_calcfg_request(_suplaclient, &request);
}

char supla_client_zwave_get_wake_up_settings(void *_suplaclient,
                                             int channelID) {
  TCS_DeviceCalCfgRequest_B request;
  memset(&request, 0, sizeof(TCS_DeviceCalCfgRequest_B));
  request.Target = SUPLA_TARGET_CHANNEL;
  request.Id = channelID;
  request.Command = SUPLA_CALCFG_CMD_ZWAVE_GET_WAKE_UP_SETTINGS;

  return supla_client_device_calcfg_request(_suplaclient, &request);
}

char supla_client_zwave_set_wake_up_time(void *_suplaclient, int channelID,
                                         unsigned int time) {
  TCS_DeviceCalCfgRequest_B request;
  memset(&request, 0, sizeof(TCS_DeviceCalCfgRequest_B));
  request.Target = SUPLA_TARGET_CHANNEL;
  request.Id = channelID;
  request.Command = SUPLA_CALCFG_CMD_ZWAVE_SET_WAKE_UP_TIME;

  ((TCalCfg_ZWave_WakeUpTime *)request.Data)->TimeSec = time;
  request.DataSize = sizeof(TCalCfg_ZWave_WakeUpTime);

  return supla_client_device_calcfg_request(_suplaclient, &request);
}

char supla_client_set_lightsource_lifespan(void *_suplaclient, int channelID,
                                           unsigned char resetCounter,
                                           unsigned char setTime,
                                           unsigned short lightSourceLifespan) {
  TCS_DeviceCalCfgRequest_B request;
  memset(&request, 0, sizeof(TCS_DeviceCalCfgRequest_B));
  request.Target = SUPLA_TARGET_CHANNEL;
  request.Id = channelID;
  request.Command = SUPLA_CALCFG_CMD_SET_LIGHTSOURCE_LIFESPAN;

  request.DataSize = sizeof(TCalCfg_LightSourceLifespan);

  TCalCfg_LightSourceLifespan *lsls =
      (TCalCfg_LightSourceLifespan *)request.Data;

  lsls->ResetCounter = resetCounter;
  lsls->SetTime = setTime;
  lsls->LightSourceLifespan = lightSourceLifespan;

  return supla_client_device_calcfg_request(_suplaclient, &request);
}

char supla_client_set_dgf_transparency(void *_suplaclient, int channelID,
                                       unsigned short mask,
                                       unsigned short active_bits) {
  TSuplaClientData *suplaclient = (TSuplaClientData *)_suplaclient;
  char result = 0;

  lck_lock(suplaclient->lck);
  if (supla_client_registered(_suplaclient) == 1) {
    if (srpc_get_proto_version(suplaclient->srpc) >= 14) {
      TCS_SuplaNewValue value;
      memset(&value, 0, sizeof(TCS_SuplaNewValue));

      ((TCSD_Digiglass_NewValue *)value.value)->mask = mask;
      ((TCSD_Digiglass_NewValue *)value.value)->active_bits = active_bits;

      value.Id = channelID;
      value.Target = SUPLA_TARGET_CHANNEL;
      result = srpc_cs_async_set_value(suplaclient->srpc, &value) ==
                       SUPLA_RESULT_FALSE
                   ? 0
                   : 1;
    }
  }
  lck_unlock(suplaclient->lck);

  return result;
}

int supla_client_get_time_diff(void *_suplaclient) {
  TSuplaClientData *suplaclient = (TSuplaClientData *)_suplaclient;
  return suplaclient->server_time_diff;
}

char supla_client_timer_arm(void *_suplaclient, int channelID, char On,
                            unsigned int durationMS) {
  TSuplaClientData *suplaclient = (TSuplaClientData *)_suplaclient;
  char result = 0;

  lck_lock(suplaclient->lck);
  if (supla_client_registered(_suplaclient) == 1) {
    TCS_TimerArmRequest request = {};
    request.ChannelID = channelID;
    request.On = On;
    request.DurationMS = durationMS;

    result = srpc_cs_async_timer_arm(suplaclient->srpc, &request) ==
                     SUPLA_RESULT_FALSE
                 ? 0
                 : 1;
  }
  lck_unlock(suplaclient->lck);
  return result;
}

char supla_client_execute_action(void *_suplaclient, int action_id, void *param,
                                 unsigned _supla_int16_t param_size,
                                 unsigned char subject_type, int subject_id) {
  TSuplaClientData *suplaclient = (TSuplaClientData *)_suplaclient;
  char result = 0;

  lck_lock(suplaclient->lck);
  if (supla_client_registered(_suplaclient) == 1) {
    TCS_Action action = {};
    action.ActionId = action_id;
    action.SubjectType = subject_type;
    action.SubjectId = subject_id;

    if (param_size && param_size <= SUPLA_ACTION_PARAM_MAXSIZE) {
      action.ParamSize = param_size;
      memcpy(action.Param, param, param_size);
    }

    result = srpc_cs_async_execute_action(suplaclient->srpc, &action) ==
                     SUPLA_RESULT_FALSE
                 ? 0
                 : 1;
  }
  lck_unlock(suplaclient->lck);
  return result;
}

char supla_client_pn_register_client_token(void *_suplaclient,
                                           TCS_RegisterPnClientToken *reg) {
  TSuplaClientData *suplaclient = (TSuplaClientData *)_suplaclient;
  char result = 0;

  lck_lock(suplaclient->lck);
  result = srpc_cs_async_register_pn_client_token(suplaclient->srpc, reg) ==
                   SUPLA_RESULT_FALSE
               ? 0
               : 1;
  lck_unlock(suplaclient->lck);
  return result;
}

char supla_client_get_channel_config(void *_suplaclient,
                                     TCS_GetChannelConfigRequest *request) {
  TSuplaClientData *suplaclient = (TSuplaClientData *)_suplaclient;
  char result = 0;

  lck_lock(suplaclient->lck);
  result = srpc_cs_async_get_channel_config_request(
               suplaclient->srpc, request) == SUPLA_RESULT_FALSE
               ? 0
               : 1;
  lck_unlock(suplaclient->lck);
  return result;
}

char supla_client_set_channel_config(void *_suplaclient,
                                     TSCS_ChannelConfig *config) {
  TSuplaClientData *suplaclient = (TSuplaClientData *)_suplaclient;
  char result = 0;

  lck_lock(suplaclient->lck);
  result = srpc_cs_async_set_channel_config_request(
               suplaclient->srpc, config) == SUPLA_RESULT_FALSE
               ? 0
               : 1;
  lck_unlock(suplaclient->lck);
  return result;
}

char supla_client_get_device_config(void *_suplaclient,
                                    TCS_GetDeviceConfigRequest *request) {
  TSuplaClientData *suplaclient = (TSuplaClientData *)_suplaclient;
  char result = 0;

  lck_lock(suplaclient->lck);
  result = srpc_cs_async_get_device_config_request(
               suplaclient->srpc, request) == SUPLA_RESULT_FALSE
               ? 0
               : 1;
  lck_unlock(suplaclient->lck);
  return result;
}
