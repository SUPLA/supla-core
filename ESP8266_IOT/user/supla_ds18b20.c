/*
 ============================================================================
 Name        : supla_ds18b20.c
 Author      : Przemyslaw Zygmunt p.zygmunt@acsoftware.pl [AC SOFTWARE]
 Version     : 1.0
 Copyright   : 2016 GPLv2
 ============================================================================
*/

// Code based on http://tech.scargill.net/esp8266-and-the-dallas-ds18b20-and-ds18b20p/

#include <os_type.h>
#include <osapi.h>
#include <gpio.h>
#include <eagle_soc.h>
#include <ets_sys.h>

#include "supla_ds18b20.h"
#include "supla-dev/log.h"

static int supla_ds18b20_gpioPin = GPIO_ID_PIN(2);
static double supla_ds18b20_last_temp = -275;

static ETSTimer supla_ds18b20_timer1;
static ETSTimer supla_ds18b20_timer2;

void ICACHE_FLASH_ATTR supla_ds18b20_init(void) {
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
    PIN_PULLUP_EN(PERIPHS_IO_MUX_GPIO2_U);
    GPIO_DIS_OUTPUT( supla_ds18b20_gpioPin );
}

void ICACHE_FLASH_ATTR supla_ds18b20_reset(void)
{
    uint8_t retries = 125;
    GPIO_DIS_OUTPUT( supla_ds18b20_gpioPin );
    do {
        if (--retries == 0) return;
        os_delay_us(2);
    } while ( !GPIO_INPUT_GET( supla_ds18b20_gpioPin ));

    GPIO_OUTPUT_SET( supla_ds18b20_gpioPin, 0 );
    os_delay_us(480);
    GPIO_DIS_OUTPUT( supla_ds18b20_gpioPin );
    os_delay_us(480);
}

void ICACHE_FLASH_ATTR supla_ds18b20_write_bit( int v )
{
    GPIO_OUTPUT_SET( supla_ds18b20_gpioPin, 0 );
    if( v ) {
        os_delay_us(10);
        GPIO_OUTPUT_SET( supla_ds18b20_gpioPin, 1 );
        os_delay_us(55);
    } else {
        os_delay_us(65);
        GPIO_OUTPUT_SET( supla_ds18b20_gpioPin, 1 );
        os_delay_us(5);
    }
}


int ICACHE_FLASH_ATTR supla_ds18b20_read_bit(void)
{
    int r;
    GPIO_OUTPUT_SET( supla_ds18b20_gpioPin, 0 );
    os_delay_us(3);
    GPIO_DIS_OUTPUT( supla_ds18b20_gpioPin );
    os_delay_us(10);
    r = GPIO_INPUT_GET( supla_ds18b20_gpioPin );
    os_delay_us(53);
    return r;
}

void ICACHE_FLASH_ATTR  supla_ds18b20_write( uint8_t v, int power ) {
    uint8_t bitMask;
    for (bitMask = 0x01; bitMask; bitMask <<= 1) {
    	supla_ds18b20_write_bit( (bitMask & v)?1:0);
    }
    if ( !power) {
        GPIO_DIS_OUTPUT( supla_ds18b20_gpioPin );
        GPIO_OUTPUT_SET( supla_ds18b20_gpioPin, 0 );
    }
}

uint8_t ICACHE_FLASH_ATTR supla_ds18b20_read() {
    uint8_t bitMask;
    uint8_t r = 0;
    for (bitMask = 0x01; bitMask; bitMask <<= 1) {
        if ( supla_ds18b20_read_bit()) r |= bitMask;
    }
    return r;
}


void ICACHE_FLASH_ATTR
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
        t = (data[0]+(data[1]*256))/16.0;

        if ( t > 1000 )
        	t-=4096;

    } else {
    	t = -275;
    }

    if ( supla_ds18b20_last_temp != t ) {
    	supla_ds18b20_last_temp = t;
    	supla_esp_devconn_on_temp_changed(t);
    };

}

void ICACHE_FLASH_ATTR
supla_ds18b20_read_temperatureA(void *timer_arg) {

	supla_ds18b20_reset();
	supla_ds18b20_write(0xcc,1);
    supla_ds18b20_write(0x44,1);

    os_timer_disarm(&supla_ds18b20_timer2);
	os_timer_setfn(&supla_ds18b20_timer2, supla_ds18b20_read_temperatureB, NULL);
    os_timer_arm (&supla_ds18b20_timer2, 1000, 0);
}

void ICACHE_FLASH_ATTR supla_ds18b20_start(void)
{
	supla_ds18b20_last_temp = -275;

	os_timer_disarm(&supla_ds18b20_timer1);
	os_timer_setfn(&supla_ds18b20_timer1, supla_ds18b20_read_temperatureA, NULL);
	os_timer_arm (&supla_ds18b20_timer1, 5000, 1);
}

void ICACHE_FLASH_ATTR supla_ds18b20_get_temp(double *value) {
	*value = supla_ds18b20_last_temp;
}

 
