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

#include <string.h>
#include <unistd.h>

#include "channel-io.h"
#include "devcfg.h"
#include "devconnection.h"
#include "log.h"
#include "srpc.h"
#include "sthread.h"
#include "supla-socket.h"
#include "tools.h"

typedef struct {
  void *ssd;
  void *srpc;
  void *sthread;

  char registered;

  struct timeval last_call;
  unsigned char server_activity_timeout;
} TDeviceConnectionData;

int devconnection_socket_read(void *buf, int count, void *dcd) {
  return ssocket_read(((TDeviceConnectionData *)dcd)->ssd, NULL, buf, count);
}

int devconnection_socket_write(void *buf, int count, void *dcd) {
  return ssocket_write(((TDeviceConnectionData *)dcd)->ssd, NULL, buf, count);
}

void devconnection_before_async_call(void *_srpc, unsigned int call_type,
                                     void *_dcd) {
  TDeviceConnectionData *dcd = (TDeviceConnectionData *)_dcd;
  gettimeofday(&dcd->last_call, NULL);
}

void devconnection_terminate(void *sthread) {
#ifdef __SINGLE_THREAD
  st_app_terminate = 1;
#else
  sthread_terminate(sthread);
#endif
}

char devconnection_isterminated(void *sthread) {
#ifdef __SINGLE_THREAD
  return st_app_terminate != 0 ? 1 : 0;
#else
  return sthread_isterminated(sthread) ? 1 : 0;
#endif
}

void devconnection_on_version_error(TDeviceConnectionData *dcd,
                                    TSDC_SuplaVersionError *version_error) {
  supla_write_state_file(scfg_string(CFG_STATE_FILE), LOG_ERR,
                         "Protocol version error. Server doesn't support this "
                         "Device. S:%d-%d/D:%d",
                         version_error->server_version_min,
                         version_error->server_version, SUPLA_PROTO_VERSION);

  devconnection_terminate(dcd->sthread);
}

void devconnection_channel_valuechanged(unsigned char number,
                                        char value[SUPLA_CHANNELVALUE_SIZE],
                                        void *_dcd) {
  srpc_ds_async_channel_value_changed(((TDeviceConnectionData *)_dcd)->srpc,
                                      number, value);
}

void devconnection_on_register_result(
    TDeviceConnectionData *dcd,
    TSD_SuplaRegisterDeviceResult *register_device_result) {
  switch (register_device_result->result_code) {
    case SUPLA_RESULTCODE_BAD_CREDENTIALS:
      supla_write_state_file(scfg_string(CFG_STATE_FILE), LOG_ERR,
                             "Bad credentials!");
      break;

    case SUPLA_RESULTCODE_TEMPORARILY_UNAVAILABLE:
      supla_write_state_file(scfg_string(CFG_STATE_FILE), LOG_NOTICE,
                             "Temporarily unavailable!");
      break;

    case SUPLA_RESULTCODE_LOCATION_CONFLICT:
      supla_write_state_file(scfg_string(CFG_STATE_FILE), LOG_ERR,
                             "Location conflict!");
      break;

    case SUPLA_RESULTCODE_CHANNEL_CONFLICT:
      supla_write_state_file(scfg_string(CFG_STATE_FILE), LOG_ERR,
                             "Channel conflict!");
      break;
    case SUPLA_RESULTCODE_TRUE:
      dcd->registered = 1;
      dcd->server_activity_timeout = register_device_result->activity_timeout;

      channelio_setcalback_on_channel_value_changed(
          &devconnection_channel_valuechanged, dcd);

      supla_write_state_file(scfg_string(CFG_STATE_FILE), LOG_DEBUG,
                             "Registered and ready.");
      break;
    case SUPLA_RESULTCODE_DEVICE_DISABLED:
      supla_write_state_file(scfg_string(CFG_STATE_FILE), LOG_NOTICE,
                             "Device is disabled!");
      break;

    case SUPLA_RESULTCODE_LOCATION_DISABLED:
      supla_write_state_file(scfg_string(CFG_STATE_FILE), LOG_NOTICE,
                             "Location is disabled!");
      break;

    case SUPLA_RESULTCODE_DEVICE_LIMITEXCEEDED:
      supla_write_state_file(scfg_string(CFG_STATE_FILE), LOG_NOTICE,
                             "Device limit exceeded!");
      break;

    case SUPLA_RESULTCODE_GUID_ERROR:
      supla_write_state_file(scfg_string(CFG_STATE_FILE), LOG_NOTICE,
                             "Incorrect device GUID!");
      break;

    case SUPLA_RESULTCODE_REGISTRATION_DISABLED:
      supla_write_state_file(scfg_string(CFG_STATE_FILE), LOG_NOTICE,
                             "Registration disabled!");
      break;

    case SUPLA_RESULTCODE_AUTHKEY_ERROR:
      supla_write_state_file(scfg_string(CFG_STATE_FILE), LOG_NOTICE,
                             "Incorrect AuthKey!");
      break;

    case SUPLA_RESULTCODE_NO_LOCATION_AVAILABLE:
      supla_write_state_file(scfg_string(CFG_STATE_FILE), LOG_NOTICE,
                             "No location available!");
      break;

    case SUPLA_RESULTCODE_USER_CONFLICT:
      supla_write_state_file(scfg_string(CFG_STATE_FILE), LOG_NOTICE,
                             "User conflict!");
      break;
  }
}

void devconnection_channel_set_value(TDeviceConnectionData *dcd,
                                     TSD_SuplaChannelNewValue *new_value) {
  if (new_value->ChannelNumber > 11) {
    // TEST
    supla_log(LOG_DEBUG,
              "Brightness: %i Color Brightness: %i R: %02X G: %02X B: %02X ",
              new_value->value[0], new_value->value[1],
              (unsigned char)new_value->value[4],
              (unsigned char)new_value->value[3],
              (unsigned char)new_value->value[2]);

    tmp_channelio_raise_valuechanged(new_value->ChannelNumber);
    srpc_ds_async_set_channel_result(dcd->srpc, new_value->ChannelNumber,
                                     new_value->SenderID, 1);

    return;
  }

  char Success =
      channelio_set_hi_value(new_value->ChannelNumber, new_value->value[0],
                             new_value->DurationMS) == 1
          ? 1
          : 0;
  srpc_ds_async_set_channel_result(dcd->srpc, new_value->ChannelNumber,
                                   new_value->SenderID, Success);
}

void devconnection_on_remote_call_received(void *_srpc, unsigned int rr_id,
                                           unsigned int call_type, void *_dcd,
                                           unsigned char proto_version) {
  TsrpcReceivedData rd;
  char result;
  TDeviceConnectionData *dcd = (TDeviceConnectionData *)_dcd;

  if (SUPLA_RESULT_TRUE == (result = srpc_getdata(_srpc, &rd, 0))) {
    switch (rd.call_type) {
      case SUPLA_SDC_CALL_VERSIONERROR:
        devconnection_on_version_error(dcd, rd.data.sdc_version_error);
        break;
      case SUPLA_SD_CALL_REGISTER_DEVICE_RESULT:
        devconnection_on_register_result(dcd,
                                         rd.data.sd_register_device_result);
        break;
      case SUPLA_SD_CALL_CHANNEL_SET_VALUE:
        devconnection_channel_set_value(dcd, rd.data.sd_channel_new_value);
        break;
    }

    srpc_rd_free(&rd);

  } else if (result == SUPLA_RESULT_DATA_ERROR) {
    supla_log(LOG_DEBUG, "DATA ERROR!");
  }
}

void devconnection_register(TDeviceConnectionData *dcd) {
  char *Email = scfg_string(CFG_EMAIL);

  if (Email == NULL || strnlen(Email, SUPLA_EMAIL_MAXSIZE) == 0) {
    TDS_SuplaRegisterDevice_C srd;
    memset(&srd, 0, sizeof(TDS_SuplaRegisterDevice_C));

    srd.channel_count = 0;
    srd.LocationID = scfg_int(CFG_LOCATION_ID);
    snprintf(srd.LocationPWD, SUPLA_LOCATION_PWD_MAXSIZE, "%s",
             scfg_string(CFG_LOCATION_PWD));
    snprintf(srd.Name, SUPLA_DEVICE_NAME_MAXSIZE, "%s",
             scfg_string(CFG_DEVNAME));
    snprintf(srd.ServerName, SUPLA_SERVER_NAME_MAXSIZE, "%s",
             scfg_string(CFG_SERVER_HOST));
    snprintf(srd.SoftVer, SUPLA_SOFTVER_MAXSIZE, "1.0");
    memcpy(srd.GUID, DEVICE_GUID, SUPLA_GUID_SIZE);

    channelio_channels_to_srd(&srd.channel_count, srd.channels);

    srpc_ds_async_registerdevice_c(dcd->srpc, &srd);

  } else {
    TDS_SuplaRegisterDevice_D srd;
    memset(&srd, 0, sizeof(TDS_SuplaRegisterDevice_D));

    srd.channel_count = 0;

    snprintf(srd.Email, SUPLA_EMAIL_MAXSIZE, "%s", Email);
    snprintf(srd.Name, SUPLA_DEVICE_NAME_MAXSIZE, "%s",
             scfg_string(CFG_DEVNAME));
    snprintf(srd.ServerName, SUPLA_SERVER_NAME_MAXSIZE, "%s",
             scfg_string(CFG_SERVER_HOST));
    snprintf(srd.SoftVer, SUPLA_SOFTVER_MAXSIZE, "1.0");

    memcpy(srd.GUID, DEVICE_GUID, SUPLA_GUID_SIZE);
    memcpy(srd.AuthKey, DEVICE_AUTHKEY, SUPLA_AUTHKEY_SIZE);

    channelio_channels_to_srd(&srd.channel_count, srd.channels);

    srpc_ds_async_registerdevice_d(dcd->srpc, &srd);
  }
}

void devconnection_ping(TDeviceConnectionData *dcd) {
  struct timeval now;

  if (dcd->server_activity_timeout > 0) {
    gettimeofday(&now, NULL);

    if ((now.tv_sec - dcd->last_call.tv_sec) >=
        (dcd->server_activity_timeout - 5)) {
      srpc_dcs_async_ping_server(dcd->srpc);
    }
  }
}

void devconnection_execute(void *user_data, void *sthread) {
  TDeviceConnectionData dcd;
  memset(&dcd, 0, sizeof(TDeviceConnectionData));

  dcd.sthread = sthread;
  char ssl = scfg_bool(CFG_SERVER_SSLENABLED);
  dcd.ssd = ssocket_client_init(
      scfg_string(CFG_SERVER_HOST),
      scfg_int(ssl == 1 ? CFG_SERVER_SSLPORT : CFG_SERVER_TCPPORT), ssl);

  if (dcd.ssd) {
    while (devconnection_isterminated(sthread) == 0) {
      supla_write_state_file(scfg_string(CFG_STATE_FILE), LOG_DEBUG,
                             "Connecting...");

      if (ssocket_client_connect(dcd.ssd, scfg_string(CFG_STATE_FILE), NULL) ==
          0) {
        usleep(5000000);

      } else {
        TEventHandler *eh = eh_init();
        TsrpcParams srpc_params;
        srpc_params_init(&srpc_params);
        srpc_params.user_params = &dcd;
        srpc_params.data_read = &devconnection_socket_read;
        srpc_params.data_write = &devconnection_socket_write;
        srpc_params.on_remote_call_received =
            &devconnection_on_remote_call_received;
        srpc_params.before_async_call = &devconnection_before_async_call;
        srpc_params.eh = eh;
        dcd.srpc = srpc_init(&srpc_params);

        eh_add_fd(eh, ssocket_get_fd(dcd.ssd));

        dcd.registered = 0;

        while (devconnection_isterminated(sthread) == 0) {
          if (dcd.registered == 0) {
            supla_write_state_file(scfg_string(CFG_STATE_FILE), LOG_DEBUG,
                                   "Register in progress...");
            dcd.registered = -1;
            devconnection_register(&dcd);

          } else if (dcd.registered == 1) {
#ifdef __SINGLE_THREAD
            channelio_iterate();
#endif

            devconnection_ping(&dcd);
          }

          if (srpc_iterate(dcd.srpc) == SUPLA_RESULT_FALSE) {
            supla_log(LOG_DEBUG, "srpc_iterate(srpc) == SUPLA_RESULT_FALSE");
            break;
          }

#ifdef __SINGLE_THREAD
          eh_wait(eh, 10000);
#else
          eh_wait(eh, 1000000);
#endif
        }

        if (dcd.registered == 1) {
          supla_write_state_file(scfg_string(CFG_STATE_FILE), LOG_DEBUG,
                                 "Disconnected");
        }

        srpc_free(dcd.srpc);
        eh_free(eh);
      }

      channelio_setcalback_on_channel_value_changed(NULL, NULL);
    }

    ssocket_free(dcd.ssd);

  } else {
    supla_log(LOG_ERR, "Can't initialize ssocket!");
  }
}

void *devconnection_start(void) {
#ifdef __SINGLE_THREAD

  devconnection_execute(NULL, NULL);
  return NULL;

#else

  Tsthread_params stp;
  stp.execute = devconnection_execute;
  stp.finish = NULL;
  stp.user_data = NULL;
  stp.free_on_finish = 0;
  stp.initialize = NULL;

  return sthread_run(&stp);

#endif
}

void devconnection_stop(void *dconn_thread) {
  if (dconn_thread != NULL) sthread_twf(dconn_thread);
}
