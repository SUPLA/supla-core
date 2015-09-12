/*
 ============================================================================
 Name        : srpc.c [SUPLA DATA EXCHANGE]
 Author      : Przemyslaw Zygmunt p.zygmunt@acsoftware.pl [AC SOFTWARE]
 Version     : 1.0
 Copyright   : GPLv2
 ============================================================================
 */

#include "srpc.h"
#include "proto.h"
#include "log.h"
#include "lck.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define srpc_BUFFER_SIZE      32768
#define srpc_QUEUE_MAX_SIZE   10

typedef struct {

   void *proto;
   TsrpcParams params;

   TSuplaDataPacket sdp;

   TSuplaDataPacket **in_queue;
   unsigned char in_queue_size;

   TSuplaDataPacket **out_queue;
   unsigned char out_queue_size;

   void *lck;

}Tsrpc;

void  srpc_params_init(TsrpcParams *params) {
    memset(params, 0, sizeof(TsrpcParams));
}

void* srpc_init(TsrpcParams *params) {

	Tsrpc *srpc = malloc(sizeof(Tsrpc));
	memset(srpc, 0, sizeof(Tsrpc));
	srpc->proto = sproto_init();

	assert(params != 0);
	assert(params->data_read != 0);
	assert(params->data_write != 0);

	memcpy(&srpc->params, params, sizeof(TsrpcParams));

	srpc->lck = lck_init();

return srpc;
}

void srpc_queue_free(TSuplaDataPacket ***queue, unsigned char *size) {

	int a;
    for(a=0;a<(*size);a++)
    	free((*queue)[a]);

    free(*queue);
    *queue = 0;
    *size = 0;

}

void srpc_free(void *_srpc) {

	if ( _srpc ) {

		Tsrpc *srpc = (Tsrpc*)_srpc;

        sproto_free(srpc->proto);
        srpc_queue_free(&srpc->in_queue, &srpc->in_queue_size);
        srpc_queue_free(&srpc->out_queue, &srpc->out_queue_size);

        lck_free(srpc->lck);

		free(srpc);
	}

}

char srpc_queue_push(TSuplaDataPacket ***queue, unsigned char *size, TSuplaDataPacket *sdp) {

	if ( *size >= srpc_QUEUE_MAX_SIZE )
		return SUPLA_RESULT_FALSE;

	(*size)++;

    *queue = realloc(*queue, sizeof(TSuplaDataPacket *)*(*size));

	if ( *queue == 0 ) {
			*size = 0;
			return SUPLA_RESULT_FALSE;
	}

	TSuplaDataPacket *sdp_new = malloc(sizeof(TSuplaDataPacket));

	if ( sdp_new == 0 )
		return SUPLA_RESULT_FALSE;

	memcpy(sdp_new, sdp, sizeof(TSuplaDataPacket));

	(*queue)[(*size)-1] = sdp_new;


	return SUPLA_RESULT_TRUE;
}

char srpc_queue_pop(TSuplaDataPacket ***queue, unsigned char *size, TSuplaDataPacket *sdp, unsigned int rr_id) {

	int a, b;

	for(a=0;a<(*size);a++)
		if ( rr_id == 0 || ((*queue)[a])->rr_id == rr_id ) {

			memcpy(sdp, (*queue)[a], sizeof(TSuplaDataPacket));

			if ( *size == 1 ) {
				srpc_queue_free(queue, size);
			} else {

				free((*queue)[a]);

                for(b=a;b<((*size)-1);b++)
                	(*queue)[b] = (*queue)[b+1];

                (*size)--;

                *queue = realloc(*queue, sizeof(TSuplaDataPacket *)*(*size));

            	if ( *queue == 0 ) {
            			*size = 0;
            			return SUPLA_RESULT_FALSE;
            	}
			}

			return SUPLA_RESULT_TRUE;
		}

	return SUPLA_RESULT_FALSE;
}

char srpc_in_queue_push(Tsrpc *srpc, TSuplaDataPacket *sdp) {
	return srpc_queue_push(&srpc->in_queue, &srpc->in_queue_size, sdp);
}

char srpc_in_queue_pop(Tsrpc *srpc, TSuplaDataPacket *sdp, unsigned int rr_id) {

	return srpc_queue_pop(&srpc->in_queue, &srpc->in_queue_size, sdp, rr_id);
}

char srpc_out_queue_push(Tsrpc *srpc, TSuplaDataPacket *sdp) {
	return srpc_queue_push(&srpc->out_queue, &srpc->out_queue_size, sdp);
}

char srpc_out_queue_pop(Tsrpc *srpc, TSuplaDataPacket *sdp, unsigned int rr_id) {
	return srpc_queue_pop(&srpc->out_queue, &srpc->out_queue_size, sdp, rr_id);
}

char srpc_iterate(void *_srpc) {

	Tsrpc *srpc = (Tsrpc*)_srpc;
	char data_buffer[srpc_BUFFER_SIZE];
	char result;
	unsigned char version;

	// --------- IN ---------------
	int data_size = srpc->params.data_read(data_buffer, srpc_BUFFER_SIZE, srpc->params.user_params);

	if ( data_size == 0 )
		return SUPLA_RESULT_FALSE;


	lck_lock(srpc->lck);

    if ( data_size > 0
    	 && SUPLA_RESULT_TRUE != ( result = sproto_in_buffer_append(srpc->proto, data_buffer, data_size) ) ) {

    	supla_log(LOG_DEBUG, "sproto_in_buffer_append: %i, datasize: %i", result, data_size);
    	return lck_unlock_r(srpc->lck, SUPLA_RESULT_FALSE);
    }

    if ( SUPLA_RESULT_TRUE == (result = sproto_pop_in_sdp(srpc->proto, &srpc->sdp) ) ) {


    	if ( SUPLA_RESULT_TRUE == srpc_in_queue_push(srpc, &srpc->sdp) ) {

    		if ( srpc->params.on_remote_call_received ) {

    			lck_unlock(srpc->lck);
    			srpc->params.on_remote_call_received(srpc, srpc->sdp.rr_id, srpc->sdp.call_type, srpc->params.user_params, srpc->sdp.version);
    			lck_lock(srpc->lck);
    		}


    	} else {
        	supla_log(LOG_DEBUG, "ssrpc_in_queue_push error");
        	return lck_unlock_r(srpc->lck, SUPLA_RESULT_FALSE);
    	}


    } else if ( result != SUPLA_RESULT_FALSE ) {

    	if ( result == SUPLA_RESULT_VERSION_ERROR ) {

    		if ( srpc->params.on_version_error ) {

    			version = srpc->sdp.version;
    			lck_unlock(srpc->lck);

    			srpc->params.on_version_error(srpc, version, srpc->params.user_params);
    			return SUPLA_RESULT_FALSE;
    		}
    	} else {
    		supla_log(LOG_DEBUG, "sproto_pop_in_sdp error: %i", result);
    	}

    	return lck_unlock_r(srpc->lck, SUPLA_RESULT_FALSE);
    }

    // --------- OUT ---------------

    if ( srpc_out_queue_pop(srpc, &srpc->sdp, 0) == SUPLA_RESULT_TRUE
    	 && SUPLA_RESULT_TRUE != (result = sproto_out_buffer_append(srpc->proto, &srpc->sdp) )
    	 && result != SUPLA_RESULT_FALSE ) {

    		supla_log(LOG_DEBUG, "sproto_out_buffer_append error: %i", result);
    		return lck_unlock_r(srpc->lck, SUPLA_RESULT_FALSE);
    	}


    data_size = sproto_pop_out_data(srpc->proto, data_buffer, srpc_BUFFER_SIZE);

    if ( data_size != 0 ) {

    	lck_unlock(srpc->lck);
    	srpc->params.data_write(data_buffer, data_size, srpc->params.user_params);
    	lck_lock(srpc->lck);

    	if ( srpc->params.eh != 0
    		 && sproto_out_dataexists(srpc->proto) == 1 ) {

    		eh_raise_event(srpc->params.eh);
    	}

    }

    return lck_unlock_r(srpc->lck, SUPLA_RESULT_TRUE);
}

// TODO srpc_getchannelpack and srpc_getlocationpack should be supported by one common function
void srpc_getchannelpack(Tsrpc *srpc, TsrpcReceivedData *rd) {

	int header_size = sizeof(TSC_SuplaChannelPack)-(sizeof(TSC_SuplaChannel)*SUPLA_CHANNELPACK_MAXSIZE);
	int c_header_size = sizeof(TSC_SuplaChannel)-SUPLA_CHANNEL_CAPTION_MAXSIZE;
	int a, count, size, offset, pack_size;
	TSC_SuplaChannelPack *pack = NULL;

	if ( srpc->sdp.data_size < header_size
		 || srpc->sdp.data_size > sizeof(TSC_SuplaChannelPack) )
		return;

	count = ((TSC_SuplaChannelPack*)srpc->sdp.data)->count;

    if ( count < 0 || count > SUPLA_CHANNELPACK_MAXSIZE )
    	return;

	pack_size = header_size+(sizeof(TSC_SuplaChannel)*count);
	pack = malloc(pack_size);
	memset(pack, 0, pack_size);
	memcpy(pack, srpc->sdp.data, header_size);

	offset = header_size;
	pack->count=0;

	for(a=0;a<count;a++)
		if ( srpc->sdp.data_size-offset >= c_header_size ) {

			size = ((TSC_SuplaChannel*)&srpc->sdp.data[offset])->CaptionSize;

			if ( size >= 0
				 && size <= SUPLA_CHANNEL_CAPTION_MAXSIZE
				 && srpc->sdp.data_size-offset >= c_header_size+size ) {

				memcpy(&pack->channels[a], &srpc->sdp.data[offset], c_header_size+size);
				offset += c_header_size+size;
				pack->count++;

			} else {
				break;
			}
    	}

	if ( count == pack->count ) {

		srpc->sdp.data_size = 0;
		rd->data.sc_channel_pack = pack;

	} else {
		free(pack);
	}
}

void srpc_getlocationpack(Tsrpc *srpc, TsrpcReceivedData *rd) {

	int header_size = sizeof(TSC_SuplaLocationPack)-(sizeof(TSC_SuplaLocation)*SUPLA_LOCATIONPACK_MAXSIZE);
	int c_header_size = sizeof(TSC_SuplaLocation)-SUPLA_LOCATION_CAPTION_MAXSIZE;
	int a, count, size, offset, pack_size;
	TSC_SuplaLocationPack *pack = NULL;

	if ( srpc->sdp.data_size < header_size
		 || srpc->sdp.data_size > sizeof(TSC_SuplaLocationPack) )
		return;

	count = ((TSC_SuplaLocationPack*)srpc->sdp.data)->count;

    if ( count < 0 || count > SUPLA_LOCATIONPACK_MAXSIZE )
    	return;

	pack_size = header_size+(sizeof(TSC_SuplaLocation)*count);
	pack = malloc(pack_size);
	memset(pack, 0, pack_size);
	memcpy(pack, srpc->sdp.data, header_size);

	offset = header_size;
	pack->count=0;

	for(a=0;a<count;a++)
		if ( srpc->sdp.data_size-offset >= c_header_size ) {

			size = ((TSC_SuplaLocation*)&srpc->sdp.data[offset])->CaptionSize;

			if ( size >= 0
				 && size <= SUPLA_LOCATION_CAPTION_MAXSIZE
				 && srpc->sdp.data_size-offset >= c_header_size+size ) {

				memcpy(&pack->locations[a], &srpc->sdp.data[offset], c_header_size+size);
				offset += c_header_size+size;
				pack->count++;

			} else {
				break;
			}
    	}

	if ( count == pack->count ) {

		srpc->sdp.data_size = 0;
		rd->data.sc_location_pack = pack;

	} else {
		free(pack);
	}
}

char srpc_getdata(void *_srpc, TsrpcReceivedData *rd, unsigned int rr_id) {

	Tsrpc *srpc = (Tsrpc*)_srpc;
	char call_with_no_data = 0;
	rd->call_type = 0;

	lck_lock(srpc->lck);

	if ( SUPLA_RESULT_TRUE == srpc_in_queue_pop(srpc, &srpc->sdp, rr_id) ) {

		rd->call_type = srpc->sdp.call_type;
		rd->rr_id = srpc->sdp.rr_id;

		// first one
		rd->data.dcs_ping = NULL;


		switch(srpc->sdp.call_type) {

		   case SUPLA_DCS_CALL_GETVERSION:
		   case SUPLA_CS_CALL_GET_NEXT:
			   call_with_no_data = 1;
			   break;

		   case SUPLA_SDC_CALL_GETVERSION_RESULT:

			   if ( srpc->sdp.data_size == sizeof(TSDC_SuplaGetVersionResult) )
                 rd->data.sdc_getversion_result = malloc(sizeof(TSDC_SuplaGetVersionResult));

			   break;

	       case SUPLA_SDC_CALL_VERSIONERROR:

			   if ( srpc->sdp.data_size == sizeof(TSDC_SuplaVersionError) )
                 rd->data.sdc_version_error = malloc(sizeof(TSDC_SuplaVersionError));

			   break;

		   case SUPLA_DCS_CALL_PING_SERVER:

			   if ( srpc->sdp.data_size == sizeof(TDCS_SuplaPingServer) )
                  rd->data.dcs_ping = malloc(sizeof(TDCS_SuplaPingServer));

			   break;

		   case SUPLA_SDC_CALL_PING_SERVER_RESULT:

			   if ( srpc->sdp.data_size == sizeof(TSDC_SuplaPingServerResult) )
                  rd->data.sdc_ping_result = malloc(sizeof(TSDC_SuplaPingServerResult));

			   break;

		   case SUPLA_DS_CALL_REGISTER_DEVICE:

			   if ( srpc->sdp.data_size >= (sizeof(TDS_SuplaRegisterDevice)-(sizeof(TDS_SuplaDeviceChannel)*SUPLA_CHANNELMAXCOUNT))
					&& srpc->sdp.data_size <= sizeof(TDS_SuplaRegisterDevice) ) {

				   rd->data.ds_register_device = malloc(sizeof(TDS_SuplaRegisterDevice));
			   }

			   break;

		   case SUPLA_SD_CALL_REGISTER_DEVICE_RESULT:

			   if ( srpc->sdp.data_size == sizeof(TSD_SuplaRegisterDeviceResult) )
                  rd->data.sd_register_device_result = malloc(sizeof(TSD_SuplaRegisterDeviceResult));
			   break;

		   case SUPLA_CS_CALL_REGISTER_CLIENT:

			   if ( srpc->sdp.data_size == sizeof(TCS_SuplaRegisterClient) )
				   rd->data.cs_register_client = malloc(sizeof(TCS_SuplaRegisterClient));

			   break;

		   case SUPLA_SC_CALL_REGISTER_CLIENT_RESULT:

			   if ( srpc->sdp.data_size == sizeof(TSC_SuplaRegisterClientResult) )
                  rd->data.sc_register_client_result = malloc(sizeof(TSC_SuplaRegisterClientResult));

			   break;

		   case SUPLA_DS_CALL_DEVICE_CHANNEL_VALUE_CHANGED:

			   if ( srpc->sdp.data_size == sizeof(TDS_SuplaDeviceChannelValue) )
				   rd->data.ds_device_channel_value = malloc(sizeof(TDS_SuplaDeviceChannelValue));

			   break;

		   case SUPLA_SD_CALL_CHANNEL_SET_VALUE:

			   if ( srpc->sdp.data_size == sizeof(TSD_SuplaChannelNewValue) )
				   rd->data.sd_channel_new_value = malloc(sizeof(TSD_SuplaChannelNewValue));

			   break;

		   case SUPLA_DS_CALL_CHANNEL_SET_VALUE_RESULT:

			   if ( srpc->sdp.data_size == sizeof(TDS_SuplaChannelNewValueResult) )
				   rd->data.ds_channel_new_value_result = malloc(sizeof(TDS_SuplaChannelNewValueResult));

			   break;

		   case SUPLA_SC_CALL_LOCATION_UPDATE:

			   if ( srpc->sdp.data_size >= (sizeof(TSC_SuplaLocation)-SUPLA_LOCATION_CAPTION_MAXSIZE)
					&& srpc->sdp.data_size <= sizeof(TSC_SuplaLocation) ) {
				   rd->data.sc_location = malloc(sizeof(TSC_SuplaLocation));
			   }

			   break;

		   case SUPLA_SC_CALL_LOCATIONPACK_UPDATE:
			   srpc_getlocationpack(srpc, rd);
			   break;

		   case SUPLA_SC_CALL_CHANNEL_UPDATE:

			   if ( srpc->sdp.data_size >= (sizeof(TSC_SuplaChannel)-SUPLA_CHANNEL_CAPTION_MAXSIZE)
					&& srpc->sdp.data_size <= sizeof(TSC_SuplaChannel) ) {
				   rd->data.sc_channel = malloc(sizeof(TSC_SuplaChannel));
			   }

			   break;

		   case SUPLA_SC_CALL_CHANNELPACK_UPDATE:
			   srpc_getchannelpack(srpc, rd);
			   break;

		   case SUPLA_SC_CALL_CHANNEL_VALUE_UPDATE:

			   if ( srpc->sdp.data_size == sizeof(TSC_SuplaChannelValue) )
				   rd->data.sc_channel_value = malloc(sizeof(TSC_SuplaChannelValue));

			   break;

		   case SUPLA_CS_CALL_CHANNEL_SET_VALUE:

			   if ( srpc->sdp.data_size == sizeof(TCS_SuplaChannelNewValue) )
				   rd->data.cs_channel_new_value = malloc(sizeof(TCS_SuplaChannelNewValue));

			   break;

		   case SUPLA_SC_CALL_EVENT:

			   if ( srpc->sdp.data_size >= (sizeof(TSC_SuplaEvent)-SUPLA_SENDER_NAME_MAXSIZE)
					&& srpc->sdp.data_size <= sizeof(TSC_SuplaEvent) ) {
				   rd->data.sc_event = malloc(sizeof(TSC_SuplaEvent));
			   }

			   break;

		}

		if ( rd->data.dcs_ping != NULL || call_with_no_data == 1 ) {

			if ( srpc->sdp.data_size > 0 ) {
				memcpy(rd->data.dcs_ping, srpc->sdp.data, srpc->sdp.data_size);
			}


			return lck_unlock_r(srpc->lck, SUPLA_RESULT_TRUE);
		}


		return lck_unlock_r(srpc->lck, SUPLA_RESULT_DATA_ERROR);
	}

	return lck_unlock_r(srpc->lck, SUPLA_RESULT_FALSE);
}

void srpc_rd_free(TsrpcReceivedData *rd) {
    if ( rd->call_type > 0 ) {
    	// first one

    	if ( rd->data.dcs_ping != NULL )
    	  free(rd->data.dcs_ping);

    	rd->call_type = 0;
    }
}

int srpc_async__call(void *_srpc, unsigned int call_type, char *data, unsigned int data_size, unsigned char *version) {

	Tsrpc *srpc = (Tsrpc*)_srpc;

	if ( srpc->params.before_async_call != NULL )
		srpc->params.before_async_call(_srpc, call_type, srpc->params.user_params);

	lck_lock(srpc->lck);

	sproto_sdp_init(srpc->proto, &srpc->sdp);

	if ( version != NULL )
		srpc->sdp.version = *version;

	if ( SUPLA_RESULT_TRUE == sproto_set_data(&srpc->sdp, data, data_size, call_type)
		 && srpc_out_queue_push(srpc, &srpc->sdp) ) {


    	if ( srpc->params.eh != 0 ) {
    		eh_raise_event(srpc->params.eh);
    	}

    	return lck_unlock_r(srpc->lck, srpc->sdp.rr_id);
	}

	return lck_unlock_r(srpc->lck, SUPLA_RESULT_FALSE);
}

int srpc_async_call(void *_srpc, unsigned int call_type, char *data, unsigned int data_size) {

	return srpc_async__call(_srpc, call_type, data, data_size, NULL);
}

unsigned char srpc_get_proto_version(void *_srpc) {

	unsigned char version;

	Tsrpc *srpc = (Tsrpc*)_srpc;
	lck_lock(srpc->lck);
	version = sproto_get_version(srpc->proto);
	lck_unlock(srpc->lck);

	return version;
}

void srpc_set_proto_version(void *_srpc, unsigned char version) {

	Tsrpc *srpc = (Tsrpc*)_srpc;

	lck_lock(srpc->lck);
	sproto_set_version(srpc->proto, version);
	lck_unlock(srpc->lck);
}

int srpc_dcs_async_getversion(void *_srpc) {
	return srpc_async_call(_srpc, SUPLA_DCS_CALL_GETVERSION, NULL, 0);
}

int srpc_sdc_async_getversion_result(void *_srpc, char *SoftVer) {

	TSDC_SuplaGetVersionResult gvr;

	gvr.proto_version = SUPLA_PROTO_VERSION;
	gvr.proto_version_min = SUPLA_PROTO_VERSION_MIN;

	memcpy(gvr.SoftVer, SoftVer, SUPLA_SOFTVER_MAXSIZE);

	return srpc_async_call(_srpc, SUPLA_SDC_CALL_GETVERSION_RESULT, (char*)&gvr, sizeof(TSDC_SuplaGetVersionResult));

}

int srpc_sdc_async_versionerror(void *_srpc, unsigned char remote_version) {
	TSDC_SuplaVersionError ve;
	ve.server_version = SUPLA_PROTO_VERSION;
	ve.server_version_min = SUPLA_PROTO_VERSION_MIN;

	return srpc_async__call(_srpc, SUPLA_SDC_CALL_VERSIONERROR, (char*)&ve, sizeof(TSDC_SuplaVersionError), &remote_version);
}

int srpc_dcs_async_ping_server(void *_srpc) {
	TDCS_SuplaPingServer ps;
	gettimeofday(&ps.now, NULL);

    return srpc_async_call(_srpc, SUPLA_DCS_CALL_PING_SERVER, (char*)&ps, sizeof(TDCS_SuplaPingServer));
}

int srpc_sdc_async_ping_server_result(void *_srpc) {
	TSDC_SuplaPingServerResult ps;
	gettimeofday(&ps.now, NULL);

	return srpc_async_call(_srpc, SUPLA_SDC_CALL_PING_SERVER_RESULT, (char*)&ps, sizeof(TSDC_SuplaPingServerResult));
}

int srpc_ds_async_registerdevice(void *_srpc, TDS_SuplaRegisterDevice *registerdevice) {

	int size = sizeof(TDS_SuplaRegisterDevice)
			- ( sizeof(TDS_SuplaDeviceChannel) * SUPLA_CHANNELMAXCOUNT )
			+ ( sizeof(TDS_SuplaDeviceChannel) * registerdevice->channel_count );

	if ( size > sizeof(TDS_SuplaRegisterDevice) )
		return 0;

	return srpc_async_call(_srpc, SUPLA_DS_CALL_REGISTER_DEVICE, (char*)registerdevice, size);
}

int srpc_sd_async_registerdevice_result(void *_srpc, TSD_SuplaRegisterDeviceResult *registerdevice_result) {
	return srpc_async_call(_srpc, SUPLA_SD_CALL_REGISTER_DEVICE_RESULT, (char*)registerdevice_result, sizeof(TSD_SuplaRegisterDeviceResult));
}

int srpc_cs_async_registerclient(void *_srpc, TCS_SuplaRegisterClient *registerclient) {
	return srpc_async_call(_srpc, SUPLA_CS_CALL_REGISTER_CLIENT, (char*)registerclient, sizeof(TCS_SuplaRegisterClient));
}

int srpc_sc_async_registerclient_result(void *_srpc, TSC_SuplaRegisterClientResult *registerclient_result) {
	return srpc_async_call(_srpc, SUPLA_SC_CALL_REGISTER_CLIENT_RESULT, (char*)registerclient_result, sizeof(TSC_SuplaRegisterClientResult));
}

int srpc_ds_async_channel_value_changed(void *_srpc, unsigned char channel_number, char *value) {
	TDS_SuplaDeviceChannelValue ncsc;
	ncsc.ChannelNumber = channel_number;
	memcpy(ncsc.value, value, SUPLA_CHANNELVALUE_SIZE);

	return srpc_async_call(_srpc, SUPLA_DS_CALL_DEVICE_CHANNEL_VALUE_CHANGED, (char*)&ncsc, sizeof(TDS_SuplaDeviceChannelValue));
}

int srpc_sd_async_set_channel_value(void *_srpc, TSD_SuplaChannelNewValue *value) {

	return srpc_async_call(_srpc, SUPLA_SD_CALL_CHANNEL_SET_VALUE, (char*)value, sizeof(TSD_SuplaChannelNewValue));

}

int srpc_ds_async_set_channel_result(void *_srpc, unsigned char ChannelNumber, int SenderID, char Success) {

	TDS_SuplaChannelNewValueResult result;
	result.ChannelNumber = ChannelNumber;
	result.SenderID = SenderID;
	result.Success = Success;

	return srpc_async_call(_srpc, SUPLA_DS_CALL_CHANNEL_SET_VALUE_RESULT, (char*)&result, sizeof(TDS_SuplaChannelNewValueResult));
}

int srpc_sc_async_location_update(void *_srpc, TSC_SuplaLocation *location) {

	int size = sizeof(TSC_SuplaLocation)-SUPLA_LOCATION_CAPTION_MAXSIZE+location->CaptionSize;

	if ( size > sizeof(TSC_SuplaLocation) )
		return 0;

	return srpc_async_call(_srpc, SUPLA_SC_CALL_LOCATION_UPDATE, (char*)location, size);
}

int srpc_sc_async_locationpack_update(void *_srpc, TSC_SuplaLocationPack *location_pack) {

	int result = 0;
	int a;
	int n=0;
	int size = 0;
	int offset = 0;


	if ( location_pack->count > SUPLA_CHANNELPACK_MAXSIZE )
		return 0;

	size = sizeof(TSC_SuplaLocationPack)-(sizeof(TSC_SuplaLocation)*SUPLA_LOCATIONPACK_MAXSIZE);
	offset = size;

	char *buffer = malloc(size);
	memcpy(buffer, location_pack, size);

	for(a=0;a<location_pack->count;a++) {

		if ( location_pack->locations[a].CaptionSize <= SUPLA_LOCATION_CAPTION_MAXSIZE ) {
			size+=sizeof(TSC_SuplaLocation)-SUPLA_LOCATION_CAPTION_MAXSIZE+location_pack->locations[a].CaptionSize;
			buffer = realloc(buffer, size);
			memcpy(&buffer[offset], &location_pack->locations[a], size-offset);
			offset+=size-offset;
			n++;
		}

	}

	((TSC_SuplaLocationPack *)buffer)->count = n;

	result = srpc_async_call(_srpc, SUPLA_SC_CALL_LOCATIONPACK_UPDATE, buffer, size);

	free(buffer);
	return result;

}

int srpc_sc_async_channel_update(void *_srpc, TSC_SuplaChannel *channel) {

	int size = sizeof(TSC_SuplaChannel)-SUPLA_CHANNEL_CAPTION_MAXSIZE+channel->CaptionSize;

	if ( size > sizeof(TSC_SuplaChannel) )
		return 0;

	return srpc_async_call(_srpc, SUPLA_SC_CALL_CHANNEL_UPDATE, (char*)channel, size);

}

// TODO srpc_sc_async_channelpack_update and srpc_sc_async_locationpack_update should be supported by one common function
int srpc_sc_async_channelpack_update(void *_srpc, TSC_SuplaChannelPack *channel_pack) {

	int result = 0;
	int a;
	int n=0;
	int size = 0;
	int offset = 0;

	if ( channel_pack->count > SUPLA_CHANNELPACK_MAXSIZE )
		return 0;

	size = sizeof(TSC_SuplaChannelPack)-(sizeof(TSC_SuplaChannel)*SUPLA_CHANNELPACK_MAXSIZE);
	offset = size;

	char *buffer = malloc(size);
	memcpy(buffer, channel_pack, size);

	for(a=0;a<channel_pack->count;a++) {

		if ( channel_pack->channels[a].CaptionSize <= SUPLA_CHANNEL_CAPTION_MAXSIZE ) {
			size+=sizeof(TSC_SuplaChannel)-SUPLA_CHANNEL_CAPTION_MAXSIZE+channel_pack->channels[a].CaptionSize;
			buffer = realloc(buffer, size);
			memcpy(&buffer[offset], &channel_pack->channels[a], size-offset);
			offset+=size-offset;
			n++;
		}

	}

	((TSC_SuplaChannelPack *)buffer)->count = n;

	result = srpc_async_call(_srpc, SUPLA_SC_CALL_CHANNELPACK_UPDATE, buffer, size);

	free(buffer);
	return result;
}

int srpc_sc_async_channel_value_update(void *_srpc, TSC_SuplaChannelValue *channel_value) {
	return srpc_async_call(_srpc, SUPLA_SC_CALL_CHANNEL_VALUE_UPDATE, (char*)channel_value, sizeof(TSC_SuplaChannelValue));
}

int srpc_cs_async_get_next(void *_srpc) {
	return srpc_async_call(_srpc, SUPLA_CS_CALL_GET_NEXT, NULL, 0);
}

int srpc_sc_async_event(void *_srpc, TSC_SuplaEvent *event) {
	int size = sizeof(TSC_SuplaEvent)-SUPLA_SENDER_NAME_MAXSIZE+event->SenderNameSize;

	if ( size > sizeof(TSC_SuplaEvent) )
		return 0;

	return srpc_async_call(_srpc, SUPLA_SC_CALL_EVENT, (char*)event, size);
}

int srpc_cs_async_set_channel_value(void *_srpc, TCS_SuplaChannelNewValue *value) {
	return srpc_async_call(_srpc, SUPLA_CS_CALL_CHANNEL_SET_VALUE, (char*)value, sizeof(TCS_SuplaChannelNewValue));
}



