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
	if (initialized)
		return 0;

	// \RST to enable operation
	gpio_port_init(reset, 0, 1);
	system("echo \"1\" | sudo tee /sys/class/gpio/gpio4/value");

	wiringPiSetup();
	mcp23008Setup(MCP23008_PIN_START, addr);

	initialized = 1;
	mcp23008_gpio_set_value(7, 1);
	return 0;
}
