/*
 ============================================================================
 Name        : supla_ds18b20.c
 Author      : Przemyslaw Zygmunt przemek@supla.org
 Version     : 1.0
 Copyright   : 2016 GPLv2
 ============================================================================
*/

// Code based on http://tech.scargill.net/esp8266-and-the-dallas-ds18b20-and-ds18b20p/

#include <os_type.h>
#include <osapi.h>
#include <eagle_soc.h>
#include <ets_sys.h>
#include <gpio.h>

#include "supla_ds18b20.h"
#include "supla_w1.h"
#include "supla-dev/log.h"
#include "supla_dht.h"

#ifdef DS18B20


static double supla_ds18b20_last_temp = -275;

ETSTimer supla_ds18b20_timer1;
ETSTimer supla_ds18b20_timer2;

#define SUPLA_DS_MODEL_UNKNOWN 0
#define SUPLA_DS_MODEL_18B20 1
#define SUPLA_DS_MODEL_1820  2

static char SUPLA_DS_MODEL = SUPLA_DS_MODEL_UNKNOWN;
static float supla_ds18b20_divider;

// TODO: Add support for multiple sensors
// TODO: Add resolution setup

void supla_ds18b20_init(void) {

	supla_w1_init();
}

void supla_ds18b20_reset(void)
{
    uint8_t retries = 125;
    GPIO_DIS_OUTPUT( supla_w1_pin );
    do {
        if (--retries == 0) return;
        os_delay_us(2);
    } while ( !GPIO_INPUT_GET( supla_w1_pin ));

    GPIO_OUTPUT_SET( supla_w1_pin, 0 );
    os_delay_us(480);
    GPIO_DIS_OUTPUT( supla_w1_pin );
    os_delay_us(480);
}

void supla_ds18b20_write_bit( int v )
{
    GPIO_OUTPUT_SET( supla_w1_pin, 0 );
    if( v ) {
        os_delay_us(10);
        GPIO_OUTPUT_SET( supla_w1_pin, 1 );
        os_delay_us(55);
    } else {
        os_delay_us(65);
        GPIO_OUTPUT_SET( supla_w1_pin, 1 );
        os_delay_us(5);
    }
}


int supla_ds18b20_read_bit(void)
{
    int r;
    GPIO_OUTPUT_SET( supla_w1_pin, 0 );
    os_delay_us(3);
    GPIO_DIS_OUTPUT( supla_w1_pin );
    os_delay_us(10);
    r = GPIO_INPUT_GET( supla_w1_pin );
    os_delay_us(53);
    return r;
}

void supla_ds18b20_write( uint8_t v, int power ) {
    uint8_t bitMask;
    for (bitMask = 0x01; bitMask; bitMask <<= 1) {
    	supla_ds18b20_write_bit( (bitMask & v)?1:0);
    }
    if ( !power) {
        GPIO_DIS_OUTPUT( supla_w1_pin );
        GPIO_OUTPUT_SET( supla_w1_pin, 0 );
    }
}

uint8_t  supla_ds18b20_read() {
    uint8_t bitMask;
    uint8_t r = 0;
    for (bitMask = 0x01; bitMask; bitMask <<= 1) {
        if ( supla_ds18b20_read_bit()) r |= bitMask;
    }
    return r;
}


void
supla_ds18b20_read_temperatureB(void *timer_arg) {

	os_timer_disarm(&supla_ds18b20_timer2);

	supla_ds18b20_reset();
	supla_ds18b20_write(0xcc,1);
	supla_ds18b20_write(0xbe,1);


    uint8_t data[8];
    int i;
    char d = 0;

    for(i=0;i<8;i++) {
    	data[i] = supla_ds18b20_read();
    	if ( data[i] != 255 ) d = 1;
    }

    double t = -275;

    if ( d == 1 ) {
    	t = ((((int8_t)data[1])<<8) | data[0]) / supla_ds18b20_divider;

		#ifdef DHTSENSOR
		os_timer_disarm(&supla_dht_timer1);
		#endif

    } else {
    	t = -275;
    }

    if ( supla_ds18b20_last_temp != t ) {
    	supla_ds18b20_last_temp = t;
    	supla_esp_devconn_on_temp_humidity_changed(t);
    };

}

void
supla_ds18b20_read_temperatureA(void *timer_arg) {

	supla_ds18b20_reset();

	if ( SUPLA_DS_MODEL == SUPLA_DS_MODEL_UNKNOWN ) {

		uint8_t data = 0;

		supla_ds18b20_write(0x33, 1);
	    data = supla_ds18b20_read();

	    if ( data == 0x28 ) {
	    	supla_ds18b20_divider = 16.0;
	    	SUPLA_DS_MODEL = SUPLA_DS_MODEL_18B20;

	    } else if ( data == 0x10 ) {
	    	supla_ds18b20_divider = 2.0;
	    	SUPLA_DS_MODEL = SUPLA_DS_MODEL_1820;
	    }


	    if ( SUPLA_DS_MODEL != SUPLA_DS_MODEL_UNKNOWN )
	    	supla_ds18b20_read_temperatureA(NULL);

	} else {

		supla_ds18b20_write(0xcc,1);
	    supla_ds18b20_write(0x44,1);

	    os_timer_disarm(&supla_ds18b20_timer2);
		os_timer_setfn(&supla_ds18b20_timer2, supla_ds18b20_read_temperatureB, NULL);
	    os_timer_arm (&supla_ds18b20_timer2, 1000, 0);

	}

}

void supla_ds18b20_start(void)
{
	supla_ds18b20_last_temp = -275;

	os_timer_disarm(&supla_ds18b20_timer1);
	os_timer_setfn(&supla_ds18b20_timer1, supla_ds18b20_read_temperatureA, NULL);
	os_timer_arm (&supla_ds18b20_timer1, 5000, 1);
}

void supla_get_temperature(char value[SUPLA_CHANNELVALUE_SIZE]) {
	// Only temperature
	memcpy(value, &supla_ds18b20_last_temp, sizeof(double));
}

#endif

 
