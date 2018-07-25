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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "client_loop.h"
#include "clientcfg.h"
#include "supla-client-lib/log.h"
#include "supla-client-lib/srpc.h"
#include "supla-client-lib/sthread.h"
#include "supla-client-lib/supla-client.h"
#include "supla-client-lib/tools.h"

void client_loop_on_registererror(void *_suplaclient, void *sthread, int code) {
  if (code == SUPLA_RESULTCODE_BAD_CREDENTIALS) {
    st_app_terminate = 1;
    sthread_terminate(sthread);

  } else {
    usleep(5000000);
  }
}

void client_loop_location_update(void *_suplaclient, void *sthread,
                                 TSC_SuplaLocation *location) {
  supla_log(LOG_DEBUG, "Location #%i: %s EOL=%i", location->Id,
            location->Caption, location->EOL);
}

void client_loop_channel_update(void *_suplaclient, void *sthread,
                                TSC_SuplaChannel_B *channel) {
  double temp;
  if (channel->Func == 40) {
    memcpy(&temp, channel->value.value, sizeof(double));
    supla_log(LOG_DEBUG, "-> Channel #%i: %f st. EOL=%i", channel->Id, temp,
              channel->EOL);
  } else {
    supla_log(
        LOG_DEBUG,
        "Channel #%i: %s LocationID=%i, Function=%i, online=%i, value[0]: %i "
        "sub_value[0;1]: %i;%i, altIcon=%i, ProtoVersion=%i, EOL=%i",
        channel->Id, channel->Caption, channel->LocationID, channel->Func,
        channel->online, channel->value.value[0], channel->value.sub_value[0],
        channel->value.sub_value[1], channel->AltIcon, channel->ProtocolVersion,
        channel->EOL);
  }
}

void client_loop_channelgroup_update(void *_suplaclient, void *sthread,
                                     TSC_SuplaChannelGroup *channel_group) {
  supla_log(LOG_DEBUG, "ChannelGroup #%i %s LocationID=%i, Function=%i EOL=%i",
            channel_group->Id, channel_group->Caption,
            channel_group->LocationID, channel_group->Func, channel_group->EOL);
}

void client_loop_channelgroup_relation_update(
    void *_suplaclient, void *sthread,
    TSC_SuplaChannelGroupRelation *channelgroup_realtion) {
  supla_log(LOG_DEBUG,
            "ChannelGroupRelation GroupId: %i ChannelId: %i, EOL: %i",
            channelgroup_realtion->ChannelGroupID,
            channelgroup_realtion->ChannelID, channelgroup_realtion->EOL);
}

void client_loop_on_event(void *_suplaclient, void *user_data,
                          TSC_SuplaEvent *event) {
  supla_log(LOG_DEBUG, "Event: %i, SenderID: %i, SenderName: %s", event->Event,
            event->SenderID, event->SenderName);
}

void client_loop_channel_value_update(void *_suplaclient, void *sthread,
                                      TSC_SuplaChannelValue *channel_value) {
  double temp;
  if (channel_value->Id == 2944) {
    TElectricityMeter_Value v;
    memcpy(&v, channel_value->value.value, sizeof(TElectricityMeter_Value));
    supla_log(LOG_DEBUG, "Channel #%i: Value: %f kWh Flags: %i",
              channel_value->Id, v.total_forward_active_energy / 100.00,
              v.flags);
  } else if (channel_value->Id == 82 || channel_value->Id == 83 ||
             channel_value->Id == 97 || channel_value->Id == 127) {
    memcpy(&temp, channel_value->value.value, sizeof(double));
    supla_log(LOG_DEBUG, "Channel #%i: %f st.", channel_value->Id, temp);
  } else {
    supla_log(
        LOG_DEBUG,
        "Channel #%i: Value: online %i, value[0]: %i, sub_value[0;1]: %i;%i, "
        "EOL: %i",
        channel_value->Id, channel_value->online, channel_value->value.value[0],
        channel_value->value.sub_value[0], channel_value->value.sub_value[1],
        channel_value->EOL);
  }
}

void client_loop_channel_extendedalue_update(
    void *_suplaclient, void *sthread,
    TSC_SuplaChannelExtendedValue *channel_extendedvalue) {
  TElectricityMeter_ExtendedValue em_ev;
  int a;
  if (srpc_evtool_v1_extended2emextended(&channel_extendedvalue->value,
                                         &em_ev) == 1) {
    supla_log(LOG_DEBUG, "*************************");
    supla_log(LOG_DEBUG, "m_count=%i", em_ev.m_count);
    supla_log(LOG_DEBUG, "measured_values=%i", em_ev.measured_values);
    supla_log(LOG_DEBUG, "period=%i sec.", em_ev.period);
    if (em_ev.m_count > 0) {
      supla_log(LOG_DEBUG, "FREQ: %i Hz", em_ev.m[0].freq / 100.00);
    }

    for (a = 0; a < 3; a++) {
      if (em_ev.m_count > 0 && em_ev.m[0].voltage[a] > 0) {
        supla_log(LOG_DEBUG, "PHASE: %i", a);
        supla_log(LOG_DEBUG, "   total_forward_active_energy=%f kW",
                  em_ev.total_forward_active_energy[a] / 100000.00);
        supla_log(LOG_DEBUG, "   total_reverse_active_energy=%f kW",
                  em_ev.total_reverse_active_energy[a] / 100000.00);
        supla_log(LOG_DEBUG, "   total_forward_reactive_energy=%f kvar",
                  em_ev.total_forward_reactive_energy[a] / 100000.00);
        supla_log(LOG_DEBUG, "   total_reverse_reactive_energy=%f kvar",
                  em_ev.total_reverse_reactive_energy[a] / 100000.00);

        supla_log(LOG_DEBUG, "   voltage=%f V", em_ev.m[0].voltage[a] / 100.00);
        supla_log(LOG_DEBUG, "   current=%f A",
                  em_ev.m[0].current[a] / 1000.00);
        supla_log(LOG_DEBUG, "   power_active=%f W",
                  em_ev.m[0].power_active[a] / 100000.00);
        supla_log(LOG_DEBUG, "   power_reactive=%f var",
                  em_ev.m[0].power_reactive[a] / 100000.00);
        supla_log(LOG_DEBUG, "   power_apparent=%f VA",
                  em_ev.m[0].power_apparent[a] / 100000.00);
        supla_log(LOG_DEBUG, "   power_factor=%f",
                  em_ev.m[0].power_factor[a] / 1000.00);
        supla_log(LOG_DEBUG, "   phase_angle=%f",
                  em_ev.m[0].phase_angle[a] / 100000.00);
      }
    }
  }
}

void client_on_registration_enabled(void *_suplaclient, void *user_data,
                                    TSDC_RegistrationEnabled *reg_enabled) {
  supla_log(LOG_DEBUG, "Client registration enabled to: %u",
            reg_enabled->client_timestamp);
  supla_log(LOG_DEBUG, "I/O Device registration enabled to: %u",
            reg_enabled->iodevice_timestamp);
}

void *client_loop_init(void *sthread) {
  TSuplaClientCfg scc;
  supla_client_cfginit(&scc);

  scc.protocol_version = proto_version;
  supla_log(LOG_DEBUG, "Proto %i", scc.protocol_version);

  snprintf(scc.Name, SUPLA_CLIENT_NAME_MAXSIZE, "Console client");
  snprintf(scc.SoftVer, SUPLA_SOFTVER_MAXSIZE, "1.0-Linux");

  if (cfg_email != NULL)
    snprintf(scc.Email, SUPLA_EMAIL_MAXSIZE, "%s", cfg_email);

  if (cfg_pwd != NULL)
    snprintf(scc.AccessIDpwd, SUPLA_ACCESSID_PWD_MAXSIZE, "%s", cfg_pwd);

  scc.AccessID = cfg_aid;
  memcpy(scc.clientGUID, cfg_client_GUID, SUPLA_GUID_SIZE);
  memcpy(scc.AuthKey, cfg_client_AuthKey, SUPLA_AUTHKEY_SIZE);

  scc.host = cfg_host;
  if (cfg_ssl_enabled) {
    scc.ssl_port = cfg_port;
  } else {
    scc.tcp_port = cfg_port;
  }
  scc.ssl_enabled = cfg_ssl_enabled;
  scc.user_data = sthread;
  scc.cb_on_registererror = &client_loop_on_registererror;
  scc.cb_location_update = &client_loop_location_update;
  scc.cb_channel_update = &client_loop_channel_update;
  scc.cb_channel_value_update = &client_loop_channel_value_update;
  scc.cb_channel_extendedvalue_update =
      &client_loop_channel_extendedalue_update;
  scc.cb_channelgroup_update = &client_loop_channelgroup_update;
  scc.cb_channelgroup_relation_update =
      &client_loop_channelgroup_relation_update;
  scc.cb_on_event = &client_loop_on_event;
  scc.cb_on_registration_enabled = &client_on_registration_enabled;

  return supla_client_init(&scc);
}

void client_loop(void *user_data, void *sthread) {
  void *sclient = client_loop_init(sthread);

  if (sclient == NULL) {
    st_app_terminate = 1;
    return;
  }

  if (user_data) *(void **)user_data = sclient;

  while (sthread_isterminated(sthread) == 0) {
    supla_log(LOG_INFO, "Connecting...");

    if (0 == supla_client_connect(sclient)) {
      usleep(2000000);
    } else {
      while (sthread_isterminated(sthread) == 0 &&
             supla_client_iterate(sclient, 10000000) == 1) {
      }
    }
  }

  if (user_data) *(void **)user_data = NULL;

  supla_client_free(sclient);
}
