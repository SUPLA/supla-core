/*
 ============================================================================
 Name        : mcp23008.h
 Author      : Jakub Palider isr@interrupt.pl []
 Version     : 1.0
 Copyright   : GPLv2
 ============================================================================
 */

#ifndef MCP23008_H_
#define MCP23008_H_

#ifdef __cplusplus
extern "C" {
#endif

#define MCP23008_PIN_START 100

char mcp23008_gpio_set_value(unsigned char port_number, char value);
char mcp23008_gpio_get_value(unsigned char port_number);
char mcp23008_gpio_port_init(unsigned char port_number, unsigned char in, char value);
char mcp23008_init(unsigned char reset, unsigned char addr );

#ifdef __cplusplus
}
#endif

#endif /* MCP23008_H_ */
