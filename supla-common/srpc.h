/*
 ============================================================================
 Name        : srpc.h [SUPLA REMOTE PROCEDURE CALL]
 Author      : Przemyslaw Zygmunt p.zygmunt@acsoftware.pl [AC SOFTWARE]
 Version     : 1.0
 Copyright   : GPLv2
 ============================================================================
 */

#ifndef supladex_H_
#define supladex_H_

#include <stdio.h>
#include <stddef.h>
#include "proto.h"
#include "eh.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*_func_srpc_DataRW)(void *buf, int count, void *user_params);
typedef void (*_func_srpc_event_OnRemoteCallReceived)(void *_srpc, unsigned int rr_id, unsigned int call_type, void *user_params, unsigned char proto_version);
typedef void (*_func_srpc_event_BeforeCall)(void *_srpc, unsigned int call_type, void *user_params);
typedef void (*_func_srpc_event_OnVersionError)(void *_srpc, unsigned char remote_version, void *user_params);

typedef struct {

	_func_srpc_DataRW data_read;
	_func_srpc_DataRW data_write;
	_func_srpc_event_OnRemoteCallReceived on_remote_call_received;
	_func_srpc_event_OnVersionError on_version_error;
	_func_srpc_event_BeforeCall before_async_call;
	TEventHandler *eh;

	void *user_params;

}TsrpcParams;

union TsrpcDataPacketData {

	TDCS_SuplaPingServer *dcs_ping;
	TSDC_SuplaPingServerResult *sdc_ping_result;
	TSDC_SuplaGetVersionResult *sdc_getversion_result;
	TSDC_SuplaVersionError *sdc_version_error;
	TDS_SuplaRegisterDevice *ds_register_device;
	TSD_SuplaRegisterDeviceResult *sd_register_device_result;
	TCS_SuplaRegisterClient *cs_register_client;
	TSC_SuplaRegisterClientResult *sc_register_client_result;
	TDS_SuplaDeviceChannelValue *ds_device_channel_value;
	TSC_SuplaLocation *sc_location;
	TSC_SuplaLocationPack *sc_location_pack;
	TSC_SuplaChannel *sc_channel;
	TSC_SuplaChannelPack *sc_channel_pack;
	TSC_SuplaChannelValue *sc_channel_value;
	TSC_SuplaEvent *sc_event;
	TSD_SuplaChannelNewValue *sd_channel_new_value;
	TDS_SuplaChannelNewValueResult *ds_channel_new_value_result;
	TCS_SuplaChannelNewValue *cs_channel_new_value;

};

typedef struct {

  unsigned int call_type;
  unsigned int rr_id;

  union TsrpcDataPacketData data;

}TsrpcReceivedData;

void  srpc_params_init(TsrpcParams *params);

void* srpc_init(TsrpcParams *params);
void srpc_free(void *_srpc);

char srpc_iterate(void *_srpc);

char srpc_getdata(void *_srpc, TsrpcReceivedData *rd, unsigned int rr_id);
void srpc_rd_free(TsrpcReceivedData *rd);

unsigned char srpc_get_proto_version(void *_srpc);
void srpc_set_proto_version(void *_srpc, unsigned char version);

// device/client <-> server
int srpc_dcs_async_getversion(void *_srpc);
int srpc_sdc_async_getversion_result(void *_srpc, char *SoftVer);
int srpc_sdc_async_versionerror(void *_srpc, unsigned char remote_version);
int srpc_dcs_async_ping_server(void *_srpc);
int srpc_sdc_async_ping_server_result(void *_srpc);

// device <-> server
int srpc_ds_async_registerdevice(void *_srpc, TDS_SuplaRegisterDevice *registerdevice);
int srpc_sd_async_registerdevice_result(void *_srpc, TSD_SuplaRegisterDeviceResult *registerdevice_result);
int srpc_ds_async_channel_value_changed(void *_srpc, unsigned char channel_number, char *value);
int srpc_sd_async_set_channel_value(void *_srpc, TSD_SuplaChannelNewValue *value);
int srpc_ds_async_set_channel_result(void *_srpc, unsigned char ChannelNumber, int SenderID, char Success);

// client <-> server
int srpc_cs_async_registerclient(void *_srpc, TCS_SuplaRegisterClient *registerclient);
int srpc_sc_async_registerclient_result(void *_srpc, TSC_SuplaRegisterClientResult *registerclient_result);
int srpc_sc_async_location_update(void *_srpc, TSC_SuplaLocation *location);
int srpc_sc_async_locationpack_update(void *_srpc, TSC_SuplaLocationPack *location_pack);
int srpc_sc_async_channel_update(void *_srpc, TSC_SuplaChannel *channel);
int srpc_sc_async_channelpack_update(void *_srpc, TSC_SuplaChannelPack *channel_pack);
int srpc_sc_async_channel_value_update(void *_srpc, TSC_SuplaChannelValue *channel_item_value);
int srpc_cs_async_get_next(void *_srpc);
int srpc_sc_async_event(void *_srpc, TSC_SuplaEvent *event);
int srpc_cs_async_set_channel_value(void *_srpc, TCS_SuplaChannelNewValue *value);

#ifdef __cplusplus
}
#endif

#endif /* supladex_H_ */
