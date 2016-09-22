/*
 ============================================================================
 Name        : supla_esp_devconn.c
 Author      : Przemyslaw Zygmunt przemek@supla.org
 Version     : 1.5
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
#include "supla_esp_gpio.h"
#include "supla_esp_cfg.h"
#include "supla_esp_pwm.h"
#include "supla_ds18b20.h"
#include "supla_dht.h"
#include "supla-dev/srpc.h"
#include "supla-dev/log.h"

static ETSTimer supla_devconn_timer1;
static ETSTimer supla_devconn_timer2;
static ETSTimer supla_devconn_timer3;
static ETSTimer supla_iterate_timer;


#if NOSSL == 1
    #define supla_espconn_sent espconn_sent
    #define supla_espconn_disconnect espconn_disconnect
    #define supla_espconn_connect espconn_connect
#else
    #define supla_espconn_sent espconn_secure_sent
	#define supla_espconn_disconnect espconn_secure_disconnect
	#define supla_espconn_connect espconn_secure_connect
#endif


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

#ifdef __BOARD_zam_row_01
#include "com/zam/supla_esp_devconn.c"
#endif

void ICACHE_FLASH_ATTR
supla_esp_system_restart(void) {

      if ( sys_restart == 1 ) {

    	  supla_log(LOG_DEBUG, "RESTART");
    	  system_print_meminfo();
    	  system_restart();
      }

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

	return supla_espconn_sent(&ESPConn, buf, count) == 0 ? count : -1;

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

		supla_esp_gpio_state_connected();

		server_activity_timeout = register_device_result->activity_timeout;
		registered = 1;

		supla_esp_set_state(LOG_DEBUG, "Registered and ready.");

		if ( server_activity_timeout != ACTIVITY_TIMEOUT ) {

			TDCS_SuplaSetActivityTimeout at;
			at.activity_timeout = ACTIVITY_TIMEOUT;
			srpc_dcs_async_set_activity_timeout(srpc, &at);

		}

		supla_esp_devconn_send_channel_values();

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
supla_esp_channel_value_changed(int channel_number, char v) {

	if ( srpc != NULL
		 && registered == 1 ) {

		//supla_log(LOG_DEBUG, "supla_esp_channel_value_changed(%i, %i)", channel_number, v);

		char value[SUPLA_CHANNELVALUE_SIZE];
		memset(value, 0, SUPLA_CHANNELVALUE_SIZE);
		value[0] = v;

		srpc_ds_async_channel_value_changed(srpc, channel_number, value);
	}

}

#if defined(RGBW_CONTROLLER_CHANNEL) \
    || defined(RGBWW_CONTROLLER_CHANNEL) \
	|| defined(RGB_CONTROLLER_CHANNEL) \
    || defined(DIMMER_CHANNEL)

void ICACHE_FLASH_ATTR
supla_esp_channel_rgbw_to_value(char value[SUPLA_CHANNELVALUE_SIZE], int color, char color_brightness, char brightness) {

	memset(value, 0, SUPLA_CHANNELVALUE_SIZE);

	value[0] = brightness;
	value[1] = color_brightness;
	value[2] = (char)((color & 0x000000FF));       // BLUE
	value[3] = (char)((color & 0x0000FF00) >> 8);  // GREEN
	value[4] = (char)((color & 0x00FF0000) >> 16); // RED

}

void ICACHE_FLASH_ATTR
supla_esp_channel_rgbw_value_changed(int channel_number, int color, char color_brightness, char brightness) {

	if ( srpc != NULL
		 && registered == 1 ) {

		char value[SUPLA_CHANNELVALUE_SIZE];

		supla_esp_channel_rgbw_to_value(value,color, color_brightness, brightness);
		srpc_ds_async_channel_value_changed(srpc, channel_number, value);
	}

}

#endif

char ICACHE_FLASH_ATTR
_supla_esp_channel_set_value(int port, char v, int channel_number) {

	char _v = v == 1 ? HI_VALUE : LO_VALUE;

	supla_esp_gpio_relay_hi(port, _v);

	_v = supla_esp_gpio_is_hi(port);

	supla_esp_channel_value_changed(channel_number, _v == HI_VALUE ? 1 : 0);

	return (v == 1 ? HI_VALUE : LO_VALUE) == _v;
}

void supla_esp_relay_timer_func(void *timer_arg) {

	_supla_esp_channel_set_value(((supla_relay_cfg_t*)timer_arg)->gpio_id, 0, ((supla_relay_cfg_t*)timer_arg)->channel);

}


#if defined(DIMMER_CHANNEL)

	char
	ICACHE_FLASH_ATTR supla_esp_channel_set_rgbw_value(int ChannelNumber, int *Color, char *ColorBrightness, char *Brightness) {

		//supla_log(LOG_DEBUG, "Color: %i, CB: %i, B: %i", *Color, *ColorBrightness, *Brightness);

		supla_esp_pwm_set_percent_duty(*Brightness, 100, 0);

		return 1;
	}

#elif defined(RGBW_CONTROLLER_CHANNEL) || defined(RGBWW_CONTROLLER_CHANNEL)

char
ICACHE_FLASH_ATTR supla_esp_channel_set_rgbw_value(int ChannelNumber, int *Color, char *ColorBrightness, char *Brightness) {

	//supla_log(LOG_DEBUG, "Color: %i, CB: %i, B: %i", *Color, *ColorBrightness, *Brightness);

	#ifdef COLOR_BRIGHTNESS_PWM

		supla_esp_pwm_set_percent_duty(*Brightness, 100, 0);
		supla_esp_pwm_set_percent_duty(*ColorBrightness, 100, 1);
		supla_esp_pwm_set_percent_duty((((*Color) & 0x00FF0000) >> 16) * 100 / 255, 100, 2); //RED
		supla_esp_pwm_set_percent_duty((((*Color) & 0x0000FF00) >> 8) * 100 / 255, 100, 3);  //GREEN
		supla_esp_pwm_set_percent_duty(((*Color) & 0x000000FF) * 100 / 255, 100, 4);         //BLUE

	#else

		int cn = 255;

		#ifdef RGBW_CONTROLLER_CHANNEL
			cn = RGBW_CONTROLLER_CHANNEL;
		#else
			cn = RGBWW_CONTROLLER_CHANNEL;
		#endif

		if ( ChannelNumber == cn ) {

			char CB = *ColorBrightness;

			if ( CB > 10 && CB < 25 )
				CB = 25;

			if ( CB < 10 )
				CB = 0;

			supla_esp_pwm_set_percent_duty((((*Color) & 0x00FF0000) >> 16) * 100 / 255, CB, 0); //RED
			supla_esp_pwm_set_percent_duty((((*Color) & 0x0000FF00) >> 8) * 100 / 255, CB, 1);  //GREEN
			supla_esp_pwm_set_percent_duty(((*Color) & 0x000000FF) * 100 / 255, CB, 2);         //BLUE

			supla_esp_pwm_set_percent_duty(*Brightness, 100, 3);

		} else if ( ChannelNumber == cn+1 ) {

			#if SUPLA_PWM_COUNT >= 4
				supla_esp_pwm_set_percent_duty(*Brightness, 100, 4);
			#endif

		}



	#endif

	return 1;
}

#elif defined(RGB_CONTROLLER_CHANNEL)

char ICACHE_FLASH_ATTR
supla_esp_channel_set_rgbw_value(int ChannelNumber, int *Color, char *ColorBrightness, char *Brightness) {

	//supla_log(LOG_DEBUG, "Color: %i, CB: %i, B: %i", *Color, *ColorBrightness, *Brightness);

	#if SUPLA_PWM_COUNT >= 4

		supla_esp_pwm_set_percent_duty(*ColorBrightness, 100, 0);
		supla_esp_pwm_set_percent_duty((((*Color) & 0x00FF0000) >> 16) * 100 / 255, 100, 1); //RED
		supla_esp_pwm_set_percent_duty((((*Color) & 0x0000FF00) >> 8) * 100 / 255, 100, 2);  //GREEN
		supla_esp_pwm_set_percent_duty(((*Color) & 0x000000FF) * 100 / 255, 100, 3);         //BLUE

	#else

		char CB = *ColorBrightness;

		if ( CB > 10 && CB < 25 )
			CB = 25;

		if ( CB < 10 )
			CB = 0;

		supla_esp_pwm_set_percent_duty((((*Color) & 0x00FF0000) >> 16) * 100 / 255, CB, 0); //RED
		supla_esp_pwm_set_percent_duty((((*Color) & 0x0000FF00) >> 8) * 100 / 255, CB, 1);  //GREEN
		supla_esp_pwm_set_percent_duty(((*Color) & 0x000000FF) * 100 / 255, CB, 2);         //BLUE

	#endif

	return 1;
}

#endif

#if defined(RGB_CONTROLLER_CHANNEL) \
    || defined(RGBW_CONTROLLER_CHANNEL) \
    || defined(RGBWW_CONTROLLER_CHANNEL) \
    || defined(DIMMER_CHANNEL)

char ICACHE_FLASH_ATTR
supla_esp_channel_set_rgbw__value(int ChannelNumber, int Color, char ColorBrightness, char Brightness) {
	supla_esp_channel_set_rgbw_value(ChannelNumber, &Color, &ColorBrightness, &Brightness);
}

#endif

void ICACHE_FLASH_ATTR
supla_esp_channel_set_value(TSD_SuplaChannelNewValue *new_value) {

#if defined(RGBW_CONTROLLER_CHANNEL) \
	|| defined(RGBWW_CONTROLLER_CHANNEL) \
	|| defined(RGB_CONTROLLER_CHANNEL) \
	|| defined(DIMMER_CHANNEL)

	unsigned char rgb_cn = 255;
	unsigned char dimmer_cn = 255;

    #ifdef RGBW_CONTROLLER_CHANNEL
	rgb_cn = RGBW_CONTROLLER_CHANNEL;
    #endif

	#ifdef RGBWW_CONTROLLER_CHANNEL
	rgb_cn = RGBWW_CONTROLLER_CHANNEL;
	dimmer_cn = RGBWW_CONTROLLER_CHANNEL+1;
	#endif

	#ifdef RGB_CONTROLLER_CHANNEL
	rgb_cn = RGB_CONTROLLER_CHANNEL;
	#endif

	#ifdef DIMMER_CHANNEL
	dimmer_cn = DIMMER_CHANNEL;
	#endif

	if ( new_value->ChannelNumber == rgb_cn
			|| new_value->ChannelNumber == dimmer_cn ) {

		int Color = 0;
		char ColorBrightness = 0;
		char Brightness = 0;

		Brightness = new_value->value[0];
		ColorBrightness = new_value->value[1];

		Color = ((int)new_value->value[4] << 16) & 0x00FF0000; // BLUE
		Color |= ((int)new_value->value[3] << 8) & 0x0000FF00; // GREEN
		Color |= (int)new_value->value[2] & 0x00000FF;         // RED

		if ( Brightness > 100 )
			Brightness = 0;

		if ( ColorBrightness > 100 )
			ColorBrightness = 0;

		supla_esp_channel_set_rgbw_value(new_value->ChannelNumber, &Color, &ColorBrightness, &Brightness);
		supla_esp_channel_rgbw_value_changed(new_value->ChannelNumber, Color, ColorBrightness, Brightness);

		return;
	}

#endif


	char v = new_value->value[0];
	int a;
	char Success = 0;

	#if defined(__BOARD_rs_module) \
		|| defined(__BOARD_rs_module_wroom) \
		|| defined(__BOARD_jangoe_rs)


		char s1, s2, v1, v2;

		v1 = 0;
		v2 = 0;

		if ( v == 1 ) {
			v1 = 1;
			v2 = 0;
		} else if ( v == 2 ) {
			v1 = 0;
			v2 = 1;
		}

		s1 = _supla_esp_channel_set_value(RELAY1_PORT, v1, new_value->ChannelNumber);
		s2 = _supla_esp_channel_set_value(RELAY2_PORT, v2, new_value->ChannelNumber);
		Success = s1 != 0 || s2 != 0;

	#else

		for(a=0;a<RELAY_MAX_COUNT;a++)
			if ( supla_relay_cfg[a].gpio_id != 0
				 && new_value->ChannelNumber == supla_relay_cfg[a].channel ) {

				Success = _supla_esp_channel_set_value(supla_relay_cfg[a].gpio_id, v, new_value->ChannelNumber);
				break;
			}


	#endif


	srpc_ds_async_set_channel_result(srpc, new_value->ChannelNumber, new_value->SenderID, Success);

	if ( v == 1 && new_value->DurationMS > 0 ) {

		for(a=0;a<RELAY_MAX_COUNT;a++)
			if ( supla_relay_cfg[a].gpio_id != 0
				 && new_value->ChannelNumber == supla_relay_cfg[a].channel ) {

				os_timer_disarm(&supla_relay_cfg[a].timer);

				os_timer_setfn(&supla_relay_cfg[a].timer, supla_esp_relay_timer_func, &supla_relay_cfg[a]);
				os_timer_arm (&supla_relay_cfg[a].timer, new_value->DurationMS, false);

				break;
			}

	}

}

void ICACHE_FLASH_ATTR
supla_esp_on_remote_call_received(void *_srpc, unsigned int rr_id, unsigned int call_type, void *_dcd, unsigned char proto_version) {

	TsrpcReceivedData rd;
	char result;

	last_response = system_get_time();

	//supla_log(LOG_DEBUG, "call_received");

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
			supla_esp_devconn_send_channel_values();
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

void
supla_esp_devconn_iterate(void *timer_arg) {

	if ( srpc != NULL ) {

		if ( registered == 0 ) {
			registered = -1;

			TDS_SuplaRegisterDevice_B srd;
			memset(&srd, 0, sizeof(TDS_SuplaRegisterDevice_B));

			srd.channel_count = 0;
			srd.LocationID = supla_esp_cfg.LocationID;
			ets_snprintf(srd.LocationPWD, SUPLA_LOCATION_PWD_MAXSIZE, "%s", supla_esp_cfg.LocationPwd);

			supla_esp_board_set_device_name(srd.Name, SUPLA_DEVICE_NAME_MAXSIZE);

			strcpy(srd.SoftVer, SUPLA_ESP_SOFTVER);
			os_memcpy(srd.GUID, supla_esp_cfg.GUID, SUPLA_GUID_SIZE);

			//supla_log(LOG_DEBUG, "LocationID=%i, LocationPWD=%s", srd.LocationID, srd.LocationPWD);

			supla_esp_board_set_channels(&srd);
/*
#if defined(__BOARD_dht11_esp01)  || \
	defined(__BOARD_dht22_esp01)  || \
    defined(__BOARD_am2302_esp01) || \
	defined(__BOARD_thermometer_esp01) || \
	defined(__BOARD_thermometer_esp01_ds_gpio0)


		srd.channel_count = 1;

		srd.channels[0].Number = 0;

		#ifdef __BOARD_dht11_esp01
		  srd.channels[0].Type = SUPLA_CHANNELTYPE_DHT11;
        #elif defined(__BOARD_dht22_esp01)
		  srd.channels[0].Type = SUPLA_CHANNELTYPE_DHT22;
        #elif defined(__BOARD_am2302_esp01)
		  srd.channels[0].Type = SUPLA_CHANNELTYPE_AM2302;
		#else
		  srd.channels[0].Type = SUPLA_CHANNELTYPE_THERMOMETERDS18B20;


        #endif
		srd.channels[0].FuncList = 0;
		srd.channels[0].Default = 0;

		supla_get_temp_and_humidity(srd.channels[0].value);


#elif defined(__BOARD_wifisocket) \
		|| defined(__BOARD_wifisocket_esp01) \
		|| defined(__BOARD_wifisocket_54) \
		|| defined(__BOARD_gate_module_esp01) \
		|| defined(__BOARD_gate_module_esp01_ds) \
		|| defined(__BOARD_sonoff) \
		|| defined(__BOARD_sonoff_ds18b20) \
		|| defined(__BOARD_jangoe_wifisocket)

            #if defined(DS18B20) || defined(DHTSENSOR)
				srd.channel_count = 2;
            #elif defined(__BOARD_jangoe_wifisocket)
				srd.channel_count = 2;
			#else
				srd.channel_count = 1;
            #endif

			srd.channels[0].Number = 0;
			srd.channels[0].Type = SUPLA_CHANNELTYPE_RELAY;

            #if defined(__BOARD_gate_module_esp01) || defined(__BOARD_gate_module_esp01_ds)
				srd.channels[0].FuncList =  SUPLA_BIT_RELAYFUNC_CONTROLLINGTHEGATEWAYLOCK \
											| SUPLA_BIT_RELAYFUNC_CONTROLLINGTHEGATE \
											| SUPLA_BIT_RELAYFUNC_CONTROLLINGTHEGARAGEDOOR \
											| SUPLA_BIT_RELAYFUNC_CONTROLLINGTHEDOORLOCK;
				srd.channels[0].Default = 0;
            #else
				srd.channels[0].FuncList = SUPLA_BIT_RELAYFUNC_POWERSWITCH \
											| SUPLA_BIT_RELAYFUNC_LIGHTSWITCH;
				srd.channels[0].Default = SUPLA_CHANNELFNC_POWERSWITCH;
            #endif


			srd.channels[0].value[0] = supla_esp_gpio_relay_on(RELAY1_PORT);

            #if defined(DS18B20) || defined(DHTSENSOR)
				srd.channels[1].Number = 1;

				#if defined(DS18B20)
					   srd.channels[1].Type = SUPLA_CHANNELTYPE_THERMOMETERDS18B20;
				#elif defined(SENSOR_DHT11)
					   srd.channels[1].Type = SUPLA_CHANNELTYPE_DHT11;
				#elif defined(SENSOR_DHT22)
					   srd.channels[1].Type = SUPLA_CHANNELTYPE_DHT22;
				#endif

			    srd.channels[1].FuncList = 0;
				srd.channels[1].Default = 0;

				supla_get_temp_and_humidity(srd.channels[1].value);

            #endif

            #ifdef __BOARD_jangoe_wifisocket
				srd.channels[1].Number = 1;
				srd.channels[1].Type = srd.channels[0].Type;
				srd.channels[1].FuncList = srd.channels[0].FuncList;
				srd.channels[1].Default = srd.channels[0].Default;
			#endif

#elif defined(__BOARD_gate_module) \
	    || defined(__BOARD_gate_module_dht11) \
	    || defined(__BOARD_gate_module_dht22) \
		|| defined(__BOARD_gate_module_wroom) \
		|| defined(__BOARD_gate_module2_wroom)

            #if defined(DS18B20) || defined(SENSOR_DHT11) || defined(SENSOR_DHT22)
				srd.channel_count = 5;
            #else
				srd.channel_count = 4;
            #endif

			srd.channels[0].Number = 0;
			srd.channels[0].Type = SUPLA_CHANNELTYPE_RELAY;
			srd.channels[0].FuncList =  SUPLA_BIT_RELAYFUNC_CONTROLLINGTHEGATEWAYLOCK \
										| SUPLA_BIT_RELAYFUNC_CONTROLLINGTHEGATE \
										| SUPLA_BIT_RELAYFUNC_CONTROLLINGTHEGARAGEDOOR \
										| SUPLA_BIT_RELAYFUNC_CONTROLLINGTHEDOORLOCK;
			srd.channels[0].Default = 0;
			srd.channels[0].value[0] = supla_esp_gpio_relay_on(RELAY1_PORT);

			srd.channels[1].Number = 1;
			srd.channels[1].Type = srd.channels[0].Type;
			srd.channels[1].FuncList = srd.channels[0].FuncList;
			srd.channels[1].Default = srd.channels[0].Default;
			srd.channels[1].value[0] = supla_esp_gpio_relay_on(RELAY2_PORT);

			srd.channels[2].Number = 2;
			srd.channels[2].Type = SUPLA_CHANNELTYPE_SENSORNO;
			srd.channels[2].FuncList = 0;
			srd.channels[2].Default = 0;
			srd.channels[2].value[0] = 0;

			srd.channels[3].Number = 3;
			srd.channels[3].Type = SUPLA_CHANNELTYPE_SENSORNO;
			srd.channels[3].FuncList = 0;
			srd.channels[3].Default = 0;
			srd.channels[3].value[0] = 0;

			// TEMPERATURE_CHANNEL
            #if defined(DS18B20) || defined(SENSOR_DHT11) || defined(SENSOR_DHT22)
				srd.channels[4].Number = 4;


                #if defined(SENSOR_DHT11)
			    	srd.channels[4].Type = SUPLA_CHANNELTYPE_DHT11;
                #elif defined(SENSOR_DHT22)
				    srd.channels[4].Type = SUPLA_CHANNELTYPE_DHT22;
                #else
				    srd.channels[4].Type = SUPLA_CHANNELTYPE_THERMOMETERDS18B20;
                #endif

				srd.channels[4].FuncList = 0;
				srd.channels[4].Default = 0;

				supla_get_temp_and_humidity(srd.channels[4].value);
            #endif

#elif defined(__BOARD_rs_module) \
		|| defined(__BOARD_rs_module_wroom) \
		|| defined(__BOARD_jangoe_rs)

            #ifdef DS18B20
				srd.channel_count = 3;
            #else
				srd.channel_count = 2;
            #endif

			srd.channels[0].Number = 0;
			srd.channels[0].Type = SUPLA_CHANNELTYPE_RELAY;
			srd.channels[0].FuncList =  SUPLA_BIT_RELAYFUNC_CONTROLLINGTHEROLLERSHUTTER;
			srd.channels[0].Default = 0;
			srd.channels[0].value[0] = supla_esp_gpio_relay_on(RELAY1_PORT) ? 1 : ( supla_esp_gpio_relay_on(RELAY2_PORT) ? 2 : 0 ) ;

			srd.channels[1].Number = 1;
			srd.channels[1].Type = SUPLA_CHANNELTYPE_SENSORNO;
			srd.channels[1].FuncList = 0;
			srd.channels[1].Default = 0;
			srd.channels[1].value[0] = 0;

            #ifdef DS18B20
				// TEMPERATURE_CHANNEL
				srd.channels[2].Number = 2;
				srd.channels[2].Type = SUPLA_CHANNELTYPE_THERMOMETERDS18B20;
				srd.channels[2].FuncList = 0;
				srd.channels[2].Default = 0;

				supla_get_temp_and_humidity(srd.channels[2].value);
            #endif

#elif defined(__BOARD_starter1_module_wroom)

			#ifdef DS18B20
				srd.channel_count = 3;
			#else
				srd.channel_count = 2;
			#endif

			srd.channels[0].Number = 0;
			srd.channels[0].Type = SUPLA_CHANNELTYPE_RELAY;
			srd.channels[0].FuncList = SUPLA_BIT_RELAYFUNC_CONTROLLINGTHEGATEWAYLOCK \
										| SUPLA_BIT_RELAYFUNC_CONTROLLINGTHEGATE \
										| SUPLA_BIT_RELAYFUNC_CONTROLLINGTHEGARAGEDOOR \
										| SUPLA_BIT_RELAYFUNC_CONTROLLINGTHEDOORLOCK;
			srd.channels[0].Default = 0;
			srd.channels[0].value[0] = supla_esp_gpio_relay_on(RELAY1_PORT);

			srd.channels[1].Number = 1;
			srd.channels[1].Type = srd.channels[0].Type;
			srd.channels[1].FuncList = srd.channels[0].FuncList;
			srd.channels[1].Default = 0;
			srd.channels[1].value[0] = supla_esp_gpio_relay_on(RELAY2_PORT);

			#ifdef DS18B20
				// TEMPERATURE_CHANNEL
				srd.channels[2].Number = 2;
				srd.channels[2].Type = SUPLA_CHANNELTYPE_THERMOMETERDS18B20;
				srd.channels[2].FuncList = 0;
				srd.channels[2].Default = 0;

				supla_get_temp_and_humidity(srd.channels[2].value);
			#endif

#elif defined(__BOARD_dimmer)

			srd.channel_count = 1;
			srd.channels[0].Type = SUPLA_CHANNELTYPE_DIMMER;
			supla_esp_channel_rgbw_to_value(srd.channels[0].value, 0x00FF00, 0, 0);
			supla_esp_channel_set_rgbw__value(0, 0x00FF00, 0, 0);


#elif defined(__BOARD_rgbw) || defined(__BOARD_rgbw_wroom)

			srd.channel_count = 1;
			srd.channels[0].Type = SUPLA_CHANNELTYPE_DIMMERANDRGBLED;
			supla_esp_channel_rgbw_to_value(srd.channels[0].value, 0x00FF00, 0, 0);
			supla_esp_channel_set_rgbw__value(0, 0x00FF00, 0, 0);

#elif defined(__BOARD_EgyIOT)

			srd.channel_count = 4;

			srd.channels[0].Number = 0;
			srd.channels[0].Type = SUPLA_CHANNELTYPE_RELAY;
			srd.channels[0].FuncList =  SUPLA_BIT_RELAYFUNC_CONTROLLINGTHEGATEWAYLOCK \
										| SUPLA_BIT_RELAYFUNC_CONTROLLINGTHEGATE \
										| SUPLA_BIT_RELAYFUNC_CONTROLLINGTHEGARAGEDOOR \
										| SUPLA_BIT_RELAYFUNC_CONTROLLINGTHEDOORLOCK \
										| SUPLA_BIT_RELAYFUNC_POWERSWITCH \
										| SUPLA_BIT_RELAYFUNC_LIGHTSWITCH;
			srd.channels[0].Default = 0;
			srd.channels[0].value[0] = supla_esp_gpio_relay_on(RELAY1_PORT);

			srd.channels[1].Number = 1;
			srd.channels[1].Type = srd.channels[0].Type;
			srd.channels[1].FuncList = srd.channels[0].FuncList;
			srd.channels[1].Default = srd.channels[0].Default;
			srd.channels[1].value[0] = supla_esp_gpio_relay_on(RELAY2_PORT);

			srd.channels[2].Number = 2;
			srd.channels[2].Type = srd.channels[0].Type;
			srd.channels[2].FuncList = srd.channels[0].FuncList;
			srd.channels[2].Default = srd.channels[0].Default;
			srd.channels[2].value[0] = supla_esp_gpio_relay_on(RELAY3_PORT);

			srd.channels[3].Number = 3;
			srd.channels[3].Type = SUPLA_CHANNELTYPE_RGBLEDCONTROLLER;

			supla_esp_channel_rgbw_to_value(srd.channels[3].value, 0x00FF00, 0, 0);
			supla_esp_channel_set_rgbw__value(3, 0x00FF00, 0, 0);

#elif defined(__BOARD_zam_row_01)

			srd.channel_count = 1;

			srd.channels[0].Number = 0;
			srd.channels[0].Type = SUPLA_CHANNELTYPE_RELAY;
			srd.channels[0].FuncList = SUPLA_BIT_RELAYFUNC_POWERSWITCH \
										| SUPLA_BIT_RELAYFUNC_LIGHTSWITCH;
			srd.channels[0].Default = SUPLA_CHANNELFNC_LIGHTSWITCH;


		    srd.channels[0].value[0] = supla_esp_gpio_relay_on(RELAY1_PORT);

#elif defined(__BOARD_h801)

			srd.channel_count = 2;
			srd.channels[0].Type = SUPLA_CHANNELTYPE_DIMMERANDRGBLED;
			supla_esp_channel_rgbw_to_value(srd.channels[0].value, 0x00FF00, 0, 0);
			supla_esp_channel_set_rgbw__value(0, 0x00FF00, 0, 0);

			srd.channels[1].Type = SUPLA_CHANNELTYPE_DIMMER;
			srd.channels[1].Number = 1;
			supla_esp_channel_rgbw_to_value(srd.channels[1].value, 0x000000, 0, 0);
			supla_esp_channel_set_rgbw__value(1, 0x000000, 0, 0);
#endif
*/
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

void
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

void
supla_esp_devconn_connect_cb(void *arg) {
	supla_log(LOG_DEBUG, "devconn_connect_cb\r\n");
	supla_esp_srpc_init();	
}

void
supla_esp_devconn_disconnect_cb(void *arg){
	supla_log(LOG_DEBUG, "devconn_disconnect_cb\r\n");
	supla_esp_system_restart();
}

void
supla_esp_devconn_delayed_disconnect_event(int sec) {

	os_timer_disarm(&supla_devconn_timer2);
	os_timer_setfn(&supla_devconn_timer2, (os_timer_func_t *)supla_esp_devconn_disconnect_cb, NULL);
	os_timer_arm(&supla_devconn_timer2, sec * 1000, 1);

}


void
supla_esp_devconn_dns_found_cb(const char *name, ip_addr_t *ip, void *arg) {

	if ( ip == NULL ) {
		supla_log(LOG_DEBUG, "Domain %s not found.", name);
		supla_esp_devconn_delayed_disconnect_event(15);
		return;

	}

	supla_espconn_disconnect(&ESPConn);

	ESPConn.proto.tcp = &ESPTCP;
	ESPConn.type = ESPCONN_TCP;
	ESPConn.state = ESPCONN_NONE;

	os_memcpy(ESPConn.proto.tcp->remote_ip, ip, 4);
	ESPConn.proto.tcp->local_port = espconn_port();

	#if NOSSL == 1
		ESPConn.proto.tcp->remote_port = 2015;
	#else
		ESPConn.proto.tcp->remote_port = 2016;
	#endif

	espconn_regist_recvcb(&ESPConn, supla_esp_devconn_recv_cb);
	espconn_regist_connectcb(&ESPConn, supla_esp_devconn_connect_cb);
	espconn_regist_disconcb(&ESPConn, supla_esp_devconn_disconnect_cb);

	supla_espconn_connect(&ESPConn);
	devconn_autoconnect = 1;

}

void
supla_esp_devconn_resolvandconnect(void) {

	devconn_autoconnect = 0;

	supla_espconn_disconnect(&ESPConn);

	uint32_t _ip = ipaddr_addr(supla_esp_cfg.Server);

	if ( _ip == -1 ) {
		 supla_log(LOG_DEBUG, "Resolv %s", supla_esp_cfg.Server);

		 espconn_gethostbyname(&ESPConn, supla_esp_cfg.Server, &ipaddr, supla_esp_devconn_dns_found_cb);
	} else {
		 supla_esp_devconn_dns_found_cb(supla_esp_cfg.Server, (ip_addr_t *)&_ip, NULL);
	}


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
	
	supla_esp_gpio_state_disconnected();

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
	os_timer_disarm(&supla_devconn_timer2);

	supla_espconn_disconnect(&ESPConn);

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

			supla_esp_gpio_state_ipreceived();

			 if ( autoconnect == 1 )
				 supla_esp_devconn_resolvandconnect();
		}


	} else {

		if ( srpc != NULL )
			supla_esp_system_restart();


		supla_esp_gpio_state_disconnected();
	}

}

void ICACHE_FLASH_ATTR
supla_esp_devconn_timer1_cb(void *timer_arg) {

	supla_esp_wifi_check_status(devconn_autoconnect);

	unsigned int t1;
	unsigned int t2;

	if ( registered == 1
		 && server_activity_timeout > 0
		 && srpc != NULL ) {

		    t1 = system_get_time();
		    t2 = abs((t1-last_response)/1000000);

		    if ( t2 >= (server_activity_timeout+10) ) {

		    	supla_log(LOG_DEBUG, "Response timeout %i, %i, %i, %i",  t1, last_response, (t1-last_response)/1000000, server_activity_timeout+5);
		    	supla_esp_system_restart();

		    } else if ( t2 >= (server_activity_timeout-5)
		    		    && t2 <= server_activity_timeout ) {

		    	//supla_log(LOG_DEBUG, "PING");
				srpc_dcs_async_ping_server(srpc);

			}

	}
}

void ICACHE_FLASH_ATTR
supla_esp_devconn_send_channel_values_cb(void *timer_arg) {

	if ( registered == 1
		 && srpc != NULL ) {

		supla_esp_board_send_channel_values(srpc);

	}

}

void ICACHE_FLASH_ATTR
supla_esp_devconn_send_channel_values(void) {

	os_timer_disarm(&supla_devconn_timer3);
	os_timer_setfn(&supla_devconn_timer3, (os_timer_func_t *)supla_esp_devconn_send_channel_values_cb, NULL);
	os_timer_arm(&supla_devconn_timer3, 2000, 0);

}

#ifdef TEMPERATURE_CHANNEL

void ICACHE_FLASH_ATTR supla_esp_devconn_on_temp_humidity_changed(char humidity) {

	if ( srpc != NULL
		 && registered == 1 ) {

		char value[SUPLA_CHANNELVALUE_SIZE];

		// Temperature or Humidity or ( Temperature and humidity )
		supla_get_temp_and_humidity(value);

		//supla_log(LOG_DEBUG, "TEMP CHANGED");
		srpc_ds_async_channel_value_changed(srpc, TEMPERATURE_CHANNEL, value);
	}

}

#endif

