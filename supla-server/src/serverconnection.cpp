/*
 ============================================================================
 Name        : serverconnection.cpp
 Author      : Przemyslaw Zygmunt p.zygmunt@acsoftware.pl [AC SOFTWARE]
 Version     : 1.0
 Copyright   : GPLv2
 ============================================================================
 */

#include <unistd.h>
#include <string.h>

#include "serverconnection.h"
#include "device.h"
#include "client.h"
#include "srpc.h"
#include "supla-socket.h"
#include "sthread.h"
#include "log.h"
#include "tools.h"
#include "svrcfg.h"

#define REGISTER_WAIT_TIMEOUT  5
#define REG_NONE    0
#define REG_DEVICE  1
#define REG_CLIENT  2

int supla_connection_socket_read(void *buf, int count, void *sc) {
	return ((serverconnection*)sc)->socket_read(buf, count);
}

int supla_connection_socket_write(void *buf, int count, void *sc) {
	return ((serverconnection*)sc)->socket_write(buf, count);
}

void supla_connection_on_remote_call_received(void *_srpc, unsigned int rr_id, unsigned int call_type, void *sc, unsigned char proto_version) {
	((serverconnection*)sc)->on_remote_call_received(_srpc, rr_id, call_type, proto_version);
}

void supla_connection_on_version_error(void *_srpc, unsigned char remote_version, void *sc) {
	int a;

	srpc_sdc_async_versionerror(_srpc, remote_version);

	for(a=0;a<20;a++)
		srpc_iterate(_srpc);
}


serverconnection::serverconnection(void *ssd, void *supla_socket, unsigned int client_ipv4) {

	gettimeofday(&this->init_time, NULL);
	this->client_ipv4 = client_ipv4;
	this->sthread = NULL;
	this->cdptr = NULL;
	this->registered = REG_NONE;
	this->ssd = ssd;
	this->supla_socket = supla_socket;

	eh = eh_init();
	eh_add_fd(eh, ssocket_supla_socket_getsfd(supla_socket));

	TsrpcParams srpc_params;
	srpc_params_init(&srpc_params);
	srpc_params.user_params = this;
	srpc_params.data_read = &supla_connection_socket_read;
	srpc_params.data_write = &supla_connection_socket_write;
	srpc_params.on_remote_call_received = &supla_connection_on_remote_call_received;
	srpc_params.on_version_error = &supla_connection_on_version_error;
	srpc_params.eh = eh;
	_srpc = srpc_init(&srpc_params);
}

serverconnection::~serverconnection() {

	if ( cdptr != NULL ) {
		if ( registered == REG_DEVICE )
			delete device;
		else
			delete client;
	}



	srpc_free(_srpc);
	eh_free(eh);
	ssocket_supla_socket_free(supla_socket);

	supla_log(LOG_DEBUG, "Connection Finished");
}

int serverconnection::socket_read(void *buf, size_t count) {
	return ssocket_read(ssd, supla_socket, buf, count);
}

int serverconnection::socket_write(const void *buf, size_t count) {
    return ssocket_write(ssd, supla_socket, buf, count);
}

void serverconnection::on_remote_call_received(void *_srpc, unsigned int rr_id, unsigned int call_type, unsigned char proto_version) {

	TsrpcReceivedData rd;

	if ( srpc_getdata(_srpc, &rd, rr_id) == 0 )
		return;

	if ( call_type == SUPLA_DCS_CALL_GETVERSION ) {

		char SoftVer[SUPLA_SOFTVER_MAXSIZE];
		memset(SoftVer, 0, SUPLA_SOFTVER_MAXSIZE);

		snprintf(SoftVer, SUPLA_SOFTVER_MAXSIZE, SERVER_VERSION);
		srpc_sdc_async_getversion_result(_srpc, SoftVer);

		goto end;
	}


	if ( registered == REG_NONE ) {

		switch(call_type) {
		case SUPLA_DS_CALL_REGISTER_DEVICE:
		case SUPLA_CS_CALL_REGISTER_CLIENT:

			if ( srpc_get_proto_version(_srpc) != proto_version ) {
				// Adjust version to client/device protocol
				supla_log(LOG_DEBUG, "Adjusting protocol version to %i", proto_version);
				srpc_set_proto_version(_srpc, proto_version);
			}
			break;
		};

		switch(call_type) {
		case SUPLA_DS_CALL_REGISTER_DEVICE:

			supla_log(LOG_DEBUG, "SUPLA_DS_CALL_REGISTER_DEVICE");

			if ( cdptr == NULL ) {
				device = new supla_device(this);
				rd.data.ds_register_device->LocationPWD[SUPLA_LOCATION_PWD_MAXSIZE-1] = 0;
				rd.data.ds_register_device->Name[SUPLA_DEVICE_NAME_MAXSIZE-1] = 0;
				rd.data.ds_register_device->SoftVer[SUPLA_SOFTVER_MAXSIZE-1] = 0;

				if ( device->register_device(rd.data.ds_register_device, proto_version) == 1 ) {
					registered = REG_DEVICE;
				};
			}

		break;

		case SUPLA_CS_CALL_REGISTER_CLIENT:

			supla_log(LOG_DEBUG, "SUPLA_CS_CALL_REGISTER_CLIENT");

			if ( cdptr == NULL ) {
				client = new supla_client(this);
				rd.data.cs_register_client->AccessIDpwd[SUPLA_ACCESSID_PWD_MAXSIZE-1] = 0;
				rd.data.cs_register_client->Name[SUPLA_CLIENT_NAME_MAXSIZE-1] = 0;
				rd.data.cs_register_client->SoftVer[SUPLA_SOFTVER_MAXSIZE-1] = 0;

				if ( client->register_client(rd.data.cs_register_client, proto_version) == 1 ) {
					registered = REG_CLIENT;
				};
			}

		break;

		default:
			sthread_terminate(sthread);
		}



	} else {

		cdptr->updateLastActivity();

		if ( call_type == SUPLA_DCS_CALL_PING_SERVER
			 && ( registered == REG_DEVICE
			      || registered == REG_CLIENT ) ) {


			srpc_sdc_async_ping_server_result(_srpc);

		} else if ( registered == REG_DEVICE ) {

			switch(call_type) {
			case SUPLA_DS_CALL_DEVICE_CHANNEL_VALUE_CHANGED:
				device->on_device_channel_value_changed(rd.data.ds_device_channel_value);
				break;

			case SUPLA_DS_CALL_CHANNEL_SET_VALUE_RESULT:
				device->on_channel_set_value_result(rd.data.ds_channel_new_value_result);
				break;
			default:

				sthread_terminate(sthread);
			}

		} else if ( registered == REG_CLIENT ) {

			switch(call_type) {
			case SUPLA_CS_CALL_GET_NEXT:
				client->get_next();
				break;
			case SUPLA_CS_CALL_CHANNEL_SET_VALUE:
				supla_log(LOG_DEBUG, "SUPLA_CS_CALL_CHANNEL_SET_VALUE");
				client->set_device_channel_new_value(rd.data.cs_channel_new_value);
			break;
			default:
				sthread_terminate(sthread);
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

	supla_log(LOG_DEBUG, "Connection Started %i, secure=%i", sthread, ssocket_is_secure(ssd));

	while(sthread_isterminated(sthread) == 0) {

		eh_wait(eh, 1000000);

		if ( srpc_iterate(_srpc) == SUPLA_RESULT_FALSE ) {
			//supla_log(LOG_DEBUG, "srpc_iterate(_srpc) == SUPLA_RESULT_FALSE");
			break;
		}

		if ( registered == REG_NONE ) {

			// TERMINATE IF REGISTRATION TIMEOUT
			struct timeval now;
			gettimeofday(&now, NULL);

			if ( now.tv_sec-init_time.tv_sec >= REGISTER_WAIT_TIMEOUT ) {
				sthread_terminate(sthread);
				supla_log(LOG_DEBUG, "Reg timeout", sthread);
				break;
			}

		} else {

			if ( cdptr->getActivityDelay() >= ACTIVITY_TIMEOUT ) {

				sthread_terminate(sthread);
				supla_log(LOG_DEBUG, "Activity timeout %i, %i, %i", sthread, cdptr->getActivityDelay(), registered);
				break;
			}

		}

	}
}

void serverconnection::terminate(void) {
	sthread_terminate(sthread);
}

unsigned int serverconnection::getClientIpv4(void) {
    return client_ipv4;
}

void* serverconnection::srpc(void) {
	return _srpc;
}
