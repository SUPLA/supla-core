/*
 ============================================================================
 Name        : supla_esp_devconn.c
 Author      : Przemyslaw Zygmunt p.zygmunt@acsoftware.pl [AC SOFTWARE]
 Version     : 1.0
 Copyright   : GPLv2
 ============================================================================
*/

#include <string.h>
#include <stdio.h>

#include <user_interface.h>
#include <espconn.h>
#include <spi_flash.h>
#include <osapi.h>
#include <mem.h>

#include "supla_esp_devconn.h"
#include "supla_esp_cfg.h"
#include "supla-dev/srpc.h"
#include "supla-dev/log.h"

static ETSTimer supla_devconn_timer1;
static ETSTimer supla_iterate_timer;

static struct espconn ESPConn;
static esp_tcp ESPTCP;
ip_addr_t ipaddr;

static void *srpc = NULL;
static char registered = 0;

static char recvbuff[RECVBUFF_MAXSIZE];
static unsigned int recvbuff_size = 0;

static char devconn_laststate[STATE_MAXSIZE];

static char devconn_autoconnect = 1;
static char sys_restart = 0;

static unsigned int last_response = 0;
static int server_activity_timeout;

static uint8 last_wifi_status = STATION_GOT_IP+1;

void ICACHE_FLASH_ATTR supla_esp_devconn_timer1_cb(void *timer_arg);
void ICACHE_FLASH_ATTR supla_esp_wifi_check_status(char autoconnect);
void ICACHE_FLASH_ATTR supla_esp_srpc_free(void);
void ICACHE_FLASH_ATTR supla_esp_devconn_iterate(void *timer_arg);


void ICACHE_FLASH_ATTR
supla_esp_system_restart(void) {

      if ( sys_restart == 1 )
    	  system_restart();
}

int ICACHE_FLASH_ATTR
supla_esp_data_read(void *buf, int count, void *dcd) {


	if ( recvbuff_size > 0 ) {

		count = recvbuff_size > count ? count : recvbuff_size;
		os_memcpy(buf, recvbuff, count);

		if ( count == recvbuff_size ) {

			recvbuff_size = 0;

		} else {

			unsigned int a;

			for(a=0;a<recvbuff_size-count;a++)
				recvbuff[a] = recvbuff[count+a];

			recvbuff_size-=count;
		}

		return count;
	}

	return -1;
}

void ICACHE_FLASH_ATTR
supla_esp_devconn_recv_cb (void *arg, char *pdata, unsigned short len) {

	if ( len == 0 || pdata == NULL )
		return;

	if ( len <= RECVBUFF_MAXSIZE-recvbuff_size ) {

		os_memcpy(&recvbuff[recvbuff_size], pdata, len);
		recvbuff_size += len;

		supla_esp_devconn_iterate(NULL);

	} else {
		supla_log(LOG_ERR, "Recv buffer size exceeded");
	}

}

int ICACHE_FLASH_ATTR
supla_esp_data_write(void *buf, int count, void *dcd) {
	return espconn_secure_sent(&ESPConn, buf, count) == 0 ? count : -1;
}


void ICACHE_FLASH_ATTR
supla_esp_set_state(int __pri, const char *message) {

	if ( message == NULL )
		return;

	unsigned char len = strlen(message)+1;

	supla_log(__pri, message);

    if ( len > STATE_MAXSIZE )
    	len = STATE_MAXSIZE;

	os_memcpy(devconn_laststate, message, len);
}

void ICACHE_FLASH_ATTR
supla_esp_on_version_error(TSDC_SuplaVersionError *version_error) {

	supla_esp_set_state(LOG_ERR, "Protocol version error");
	supla_esp_devconn_stop();
}


void ICACHE_FLASH_ATTR
supla_esp_on_register_result(TSD_SuplaRegisterDeviceResult *register_device_result) {

	switch(register_device_result->result_code) {
	case SUPLA_RESULTCODE_BAD_CREDENTIALS:
		supla_esp_set_state(LOG_ERR, "Bad credentials!");
		break;

	case SUPLA_RESULTCODE_TEMPORARILY_UNAVAILABLE:
		supla_esp_set_state(LOG_NOTICE, "Temporarily unavailable!");
		supla_esp_system_restart();
		break;

	case SUPLA_RESULTCODE_LOCATION_CONFLICT:
		supla_esp_set_state(LOG_ERR, "Location conflict!");
		break;

	case SUPLA_RESULTCODE_CHANNEL_CONFLICT:
		supla_esp_set_state(LOG_ERR, "Channel conflict!");
		break;
	case SUPLA_RESULTCODE_TRUE:

		supla_esp_gpio_led_red_on(0);
		supla_esp_gpio_led_green_on(1);
		supla_esp_gpio_led_blue_on(0);

		server_activity_timeout = register_device_result->activity_timeout;
		registered = 1;

		supla_esp_set_state(LOG_DEBUG, "Registered and ready.");

		if ( server_activity_timeout != ACTIVITY_TIMEOUT ) {

			TDCS_SuplaSetActivityTimeout at;
			at.activity_timeout = ACTIVITY_TIMEOUT;
			srpc_dcs_async_set_activity_timeout(srpc, &at);

		}

		return;

	case SUPLA_RESULTCODE_DEVICE_DISABLED:
		supla_esp_set_state(LOG_NOTICE, "Device is disabled!");
		supla_esp_system_restart();
		break;

	case SUPLA_RESULTCODE_LOCATION_DISABLED:
		supla_esp_set_state(LOG_NOTICE, "Location is disabled!");
		supla_esp_system_restart();
		break;

	case SUPLA_RESULTCODE_DEVICE_LIMITEXCEEDED:
		supla_esp_set_state(LOG_NOTICE, "Device limit exceeded!");
		break;

	case SUPLA_RESULTCODE_GUID_ERROR:
		supla_esp_set_state(LOG_NOTICE, "Incorrect device GUID!");
		break;
	}

	devconn_autoconnect = 0;
	supla_esp_devconn_stop();
}

void ICACHE_FLASH_ATTR
supla_esp_channel_set_activity_timeout_result(TSDC_SuplaSetActivityTimeoutResult *result) {
	server_activity_timeout = result->activity_timeout;
}

void ICACHE_FLASH_ATTR
supla_esp_channel_set_value(TSD_SuplaChannelNewValue *new_value) {


	char Success = 0;
	char v = new_value->value[0] == 0 ? 0 : 1;

	if ( new_value->ChannelNumber == 0 ) {


		supla_esp_gpio_hi(RELAY1_PORT, v);
		Success = supla_esp_gpio_is_hi(RELAY1_PORT) == v;

		if ( Success ) {

			char value[SUPLA_CHANNELVALUE_SIZE];
			memset(value, 0, SUPLA_CHANNELVALUE_SIZE);
			value[0] = v;

			srpc_ds_async_channel_value_changed(srpc, 0, value);
		}

	}

	srpc_ds_async_set_channel_result(srpc, new_value->ChannelNumber, new_value->SenderID, Success);
}

void ICACHE_FLASH_ATTR
supla_esp_on_remote_call_received(void *_srpc, unsigned int rr_id, unsigned int call_type, void *_dcd, unsigned char proto_version) {

	TsrpcReceivedData rd;
	char result;

	last_response = system_get_time();

	if ( SUPLA_RESULT_TRUE == ( result = srpc_getdata(_srpc, &rd, 0)) ) {

		switch(rd.call_type) {
		case SUPLA_SDC_CALL_VERSIONERROR:
			supla_esp_on_version_error(rd.data.sdc_version_error);
			break;
		case SUPLA_SD_CALL_REGISTER_DEVICE_RESULT:
			supla_esp_on_register_result(rd.data.sd_register_device_result);
			break;
		case SUPLA_SD_CALL_CHANNEL_SET_VALUE:
			supla_esp_channel_set_value(rd.data.sd_channel_new_value);
			break;
		case SUPLA_SDC_CALL_SET_ACTIVITY_TIMEOUT_RESULT:
			supla_esp_channel_set_activity_timeout_result(rd.data.sdc_set_activity_timeout_result);
			break;
		}

		srpc_rd_free(&rd);

	} else if ( result == SUPLA_RESULT_DATA_ERROR ) {

		supla_log(LOG_DEBUG, "DATA ERROR!");
	}

}

void ICACHE_FLASH_ATTR
supla_esp_devconn_iterate(void *timer_arg) {

	if ( srpc != NULL ) {

		if ( registered == 0 ) {
			registered = -1;

			TDS_SuplaRegisterDevice_B srd;
			memset(&srd, 0, sizeof(TDS_SuplaRegisterDevice_B));

			srd.channel_count = 0;
			srd.LocationID = supla_esp_cfg.LocationID;
			ets_snprintf(srd.LocationPWD, SUPLA_LOCATION_PWD_MAXSIZE, "%s", supla_esp_cfg.LocationPwd);
			ets_snprintf(srd.Name, SUPLA_DEVICE_NAME_MAXSIZE, "%s", DEVICE_NAME);
			strcpy(srd.SoftVer, "1.0");
			os_memcpy(srd.GUID, supla_esp_cfg.GUID, SUPLA_GUID_SIZE);

			srd.channel_count = 1;
			srd.channels[0].Number = 0;
			srd.channels[0].Type = CHANNEL_TYPE;
			srd.channels[0].FuncList = CHANNEL_FUNCLIST;
			srd.channels[0].Default = CHANNEL_DEFAULT_FUNC;
			srd.channels[0].value[0] = supla_esp_gpio_is_hi(RELAY1_PORT);

			srpc_ds_async_registerdevice_b(srpc, &srd);

		};

		if( srpc_iterate(srpc) == SUPLA_RESULT_FALSE ) {
			supla_log(LOG_DEBUG, "iterate fail");
			supla_esp_system_restart();
		}

	}

}


void ICACHE_FLASH_ATTR
supla_esp_srpc_free(void) {

	os_timer_disarm(&supla_iterate_timer);

	registered = 0;
	last_response = 0;

	if ( srpc != NULL ) {
		srpc_free(srpc);
		srpc = NULL;
	}
}

void ICACHE_FLASH_ATTR
supla_esp_srpc_init(void) {
	
	supla_esp_srpc_free();
		
	TsrpcParams srpc_params;
	srpc_params_init(&srpc_params);
	srpc_params.data_read = &supla_esp_data_read;
	srpc_params.data_write = &supla_esp_data_write;
	srpc_params.on_remote_call_received = &supla_esp_on_remote_call_received;

	srpc = srpc_init(&srpc_params);
	
	os_timer_setfn(&supla_iterate_timer, (os_timer_func_t *)supla_esp_devconn_iterate, NULL);
	os_timer_arm(&supla_iterate_timer, 100, 1);

}

void ICACHE_FLASH_ATTR
supla_esp_devconn_connect_cb(void *arg) {
	supla_log(LOG_DEBUG, "devconn_connect_cb\r\n");
	supla_esp_srpc_init();	
}

void ICACHE_FLASH_ATTR
supla_esp_devconn_disconnect_cb(void *arg){
	supla_log(LOG_DEBUG, "devconn_disconnect_cb\r\n");
	supla_esp_system_restart();
}


void ICACHE_FLASH_ATTR
supla_esp_devconn_dns_found_cb(const char *name, ip_addr_t *ip, void *arg) {

	if ( ip != NULL ) {

		//supla_log(LOG_DEBUG, "Domain %s found", name);

		espconn_secure_disconnect(&ESPConn);

		ESPConn.proto.tcp = &ESPTCP;
		ESPConn.type = ESPCONN_TCP;
		ESPConn.state = ESPCONN_NONE;

		os_memcpy(ESPConn.proto.tcp->remote_ip, ip, 4);
		ESPConn.proto.tcp->local_port = espconn_port();
		ESPConn.proto.tcp->remote_port = 2016;

		espconn_regist_recvcb(&ESPConn, supla_esp_devconn_recv_cb);
		espconn_regist_connectcb(&ESPConn, supla_esp_devconn_connect_cb);
		espconn_regist_disconcb(&ESPConn, supla_esp_devconn_disconnect_cb);

		espconn_secure_connect(&ESPConn);

	}

	devconn_autoconnect = 1;
}

void ICACHE_FLASH_ATTR
supla_esp_devconn_resolvandconnect(void) {

	devconn_autoconnect = 0;
	espconn_secure_disconnect(&ESPConn);
    espconn_gethostbyname(&ESPConn, supla_esp_cfg.Server, &ipaddr, supla_esp_devconn_dns_found_cb);

}

void ICACHE_FLASH_ATTR
supla_esp_devconn_init(void) {

	devconn_laststate[0] = '-';
	devconn_laststate[1] = 0;
}

void ICACHE_FLASH_ATTR
supla_esp_devconn_start(void) {
	
	sys_restart = 0;

	wifi_station_disconnect();
	
    supla_esp_gpio_led_blue_blinking(0);
    supla_esp_gpio_led_red_on(1);

    struct station_config stationConf;

    wifi_set_opmode( STATION_MODE );
    
    os_memcpy(stationConf.ssid, supla_esp_cfg.WIFI_SSID, WIFI_SSID_MAXSIZE);
    os_memcpy(stationConf.password, supla_esp_cfg.WIFI_PWD, WIFI_PWD_MAXSIZE);
   
    stationConf.ssid[31] = 0;
    stationConf.password[63] = 0;
    
    
    wifi_station_set_config(&stationConf);
    wifi_station_set_auto_connect(1);
    wifi_station_connect();
    
	os_timer_disarm(&supla_devconn_timer1);
	os_timer_setfn(&supla_devconn_timer1, (os_timer_func_t *)supla_esp_devconn_timer1_cb, NULL);
	os_timer_arm(&supla_devconn_timer1, 1000, 1);
	
	sys_restart = 1;
}

void ICACHE_FLASH_ATTR
supla_esp_devconn_stop(void) {
	
	sys_restart = 0;

	os_timer_disarm(&supla_devconn_timer1);
	espconn_secure_disconnect(&ESPConn);

	supla_esp_wifi_check_status(0);
}

char * ICACHE_FLASH_ATTR
supla_esp_devconn_laststate(void) {
	return devconn_laststate;
}

void ICACHE_FLASH_ATTR
supla_esp_wifi_check_status(char autoconnect) {

	uint8 status = wifi_station_get_connect_status();

	if ( status != last_wifi_status )
		supla_log(LOG_DEBUG, "WiFi Status: %i", status);

	last_wifi_status = status;

	if ( STATION_GOT_IP == status ) {

		if ( srpc == NULL ) {

			supla_esp_gpio_led_red_on(0);
			supla_esp_gpio_led_green_on(0);
			supla_esp_gpio_led_blue_on(1);

			 if ( autoconnect == 1 )
				 supla_esp_devconn_resolvandconnect();
		}


	} else {

		if ( srpc != NULL )
			supla_esp_system_restart();


		supla_esp_gpio_led_red_on(1);
		supla_esp_gpio_led_green_on(0);
		supla_esp_gpio_led_blue_on(0);
	}

}

void ICACHE_FLASH_ATTR
supla_esp_devconn_timer1_cb(void *timer_arg) {

	supla_esp_wifi_check_status(devconn_autoconnect);

	unsigned int t;

	if ( registered == 1
		 && server_activity_timeout > 0
		 && srpc != NULL ) {

		    t = system_get_time();

		    if ( abs((t-last_response)/1000000) >= (server_activity_timeout+10) ) {

		    	supla_log(LOG_DEBUG, "Response timeout %i, %i, %i, %i", t, last_response, (t-last_response)/1000000, server_activity_timeout+5);
		    	supla_esp_system_restart();

		    } else if ( abs((t-last_response)/1000000) >= (server_activity_timeout-5) ) {

				srpc_dcs_async_ping_server(srpc);

			}

	}
}

void ICACHE_FLASH_ATTR
supla_esp_devconn_on_port_value_changed(int port, char hi) {

	if ( srpc != NULL
		 && registered == 1 ) {

		char value[SUPLA_CHANNELVALUE_SIZE];
		memset(value, 0, SUPLA_CHANNELVALUE_SIZE);

		value[0] = hi;

		srpc_ds_async_channel_value_changed(srpc, 0, value);
		supla_esp_devconn_iterate(NULL);
	}

}
