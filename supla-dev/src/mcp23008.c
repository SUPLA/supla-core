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

#ifndef __MCP23008

char mcp23008_gpio_set_value(unsigned char port_number, char value) {
  return 0;
}

char mcp23008_gpio_get_value(unsigned char port_number) { return 0; }

char mcp23008_gpio_port_init(unsigned char port_number, unsigned char in,
                             char value) {
  return 0;
}

char mcp23008_init(unsigned char reset, unsigned char addr) { return 0; }
#else

#include "mcp23008.h"
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include "gpio.h"

char mcp23008_gpio_set_value(unsigned char port_number, char value) {
  pinMode(MCP23008_PIN_START + port_number, OUTPUT);
  digitalWrite(MCP23008_PIN_START + port_number, value);
  return 0;
}

char mcp23008_gpio_get_value(unsigned char port_number) {
  pinMode(MCP23008_PIN_START + port_number, INPUT);
  return digitalRead(MCP23008_PIN_START + port_number);
}

char mcp23008_gpio_port_init(unsigned char port_number, unsigned char in,
                             char value) {
  int port = MCP23008_PIN_START + port_number;

  pinMode(port, in ? INPUT : OUTPUT);

  if (in && value) pullUpDnControl(port, PUD_UP);

  if (!in) digitalWrite(MCP23008_PIN_START + port_number, value);

  return 0;
}

char mcp23008_init(unsigned char reset, unsigned char addr) {
  static int initialized = 0;
  int i;

  if (initialized) return 0;

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

#endif
