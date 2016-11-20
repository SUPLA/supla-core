/*
 ============================================================================
 Name        : supla_dht.c
 Author      : Przemyslaw Zygmunt przemek@supla.org
 Version     : 1.0
 Copyright   : 2016 GPLv2
 ============================================================================
*/

#include <os_type.h>
#include <osapi.h>
#include <eagle_soc.h>
#include <ets_sys.h>

#include "supla_dht.h"
#include "supla_w1.h"
#include "supla-dev/log.h"
#include "driver/dht22.h"
#include "supla_ds18b20.h"

#ifdef DHTSENSOR

static double supla_dht_last_temp = -275;
static double supla_dht_last_humidity = -1;

ETSTimer supla_dht_timer1;

void supla_dht_init(void) {
	supla_w1_init();
}

void
supla_dht_read_th(void *timer_arg) {
	
	DHT_Sensor sensor;
	DHT_Sensor_Data output;
	
	#ifdef W1_GPIO0
	  sensor.port = 0;
	#elif defined(W1_GPIO5)
	  sensor.port = 5;
	#elif defined(W1_GPIO14)
	  sensor.port = 14;
	#else
	  sensor.port = 2;
	#endif
	
    #ifdef SENSOR_DHT11
	  sensor.type = DHT11;
	#else
	  sensor.type = DHT22;
	#endif
	  
	  
	if ( DHTRead(&sensor, &output) ) {
		
		if ( supla_dht_last_temp != output.temperature
				|| supla_dht_last_humidity != output.humidity ) {
			
			supla_dht_last_temp = output.temperature;
			supla_dht_last_humidity = output.humidity;
			supla_esp_devconn_on_temp_humidity_changed(1);

            #ifdef DS18B20
			os_timer_disarm(&supla_ds18b20_timer1);
			os_timer_disarm(&supla_ds18b20_timer2);
			#endif

		}
		
		
	} else {
		supla_dht_last_temp = -275;
		supla_dht_last_humidity = -1;

	}
}

void supla_get_temp_and_humidity(char value[SUPLA_CHANNELVALUE_SIZE]) {

	int t = supla_dht_last_temp*1000.00;
	int h = supla_dht_last_humidity*1000.00;

	memcpy(value, &t, 4);
	memcpy(&value[4], &h, 4);
	
}

void  supla_dht_start(void) {
	supla_dht_last_temp = -275;
	supla_dht_last_humidity = -1;

	os_timer_disarm(&supla_dht_timer1);
	os_timer_setfn(&supla_dht_timer1, supla_dht_read_th, NULL);
	os_timer_arm (&supla_dht_timer1, 6000, 1);
}


#endif
