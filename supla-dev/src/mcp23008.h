/*
 Copyright (C) AC SOFTWARE SP. Z O.O.

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef MCP23008_H_
#define MCP23008_H_

#ifdef __cplusplus
extern "C" {
#endif

#define MCP23008_PIN_START 100
#define MCP23008_MAX_GPIO 8
#define MCP23008_GPIO_OUT 0
#define MCP23008_GPIO_IN 1

char mcp23008_gpio_set_value(unsigned char port_number, char value);
char mcp23008_gpio_get_value(unsigned char port_number);
char mcp23008_gpio_port_init(unsigned char port_number, unsigned char in,
                             char value);
char mcp23008_init(unsigned char reset, unsigned char addr);

#ifdef __cplusplus
}
#endif

#endif /* MCP23008_H_ */
