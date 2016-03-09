/*
 ============================================================================
 Name        : mcp23008.c
 Author      : Jakub Palider isr@interrupt.pl []
 Version     : 1.0
 Copyright   : GPLv2
 ============================================================================
 */


#include <stdio.h>
#include <wiringPiI2C.h>
#include <wiringPi.h>
#include <mcp23008.h>

#include "gpio.h"
#include "mcp23008.h"

char mcp23008_gpio_set_value(unsigned char port_number, char value) {

	pinMode(MCP23008_PIN_START + port_number, OUTPUT);
	digitalWrite(MCP23008_PIN_START + port_number, value);
	return 0;
}

char mcp23008_gpio_get_value(unsigned char port_number) {

	pinMode(MCP23008_PIN_START + port_number, INPUT);
	return digitalRead(MCP23008_PIN_START + port_number);
}

char mcp23008_gpio_port_init(unsigned char port_number, unsigned char in, char value) {

	pinMode(MCP23008_PIN_START + port_number, in ? INPUT : OUTPUT);
	if ( !in )
		digitalWrite(MCP23008_PIN_START + port_number, value);
	return 0;
}

char mcp23008_init(unsigned char reset, unsigned char addr ) {

	static int initialized = 0;
	int i;

	if (initialized)
		return 0;

	gpio_port_init(reset, 0, 0);
	wiringPiSetup();

	// \RST to enable MCP23008 operation
	gpio_set_value(reset, 1);
	mcp23008Setup(MCP23008_PIN_START, addr);

	for (i = 0; i < MCP23008_MAX_GPIO; i++) {
		mcp23008_gpio_port_init(i, MCP23008_GPIO_IN, 0);
	}

	initialized = 1;
	return 0;
}
