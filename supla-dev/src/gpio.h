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

#ifndef GPIO_H_
#define GPIO_H_

#ifdef __cplusplus
extern "C" {
#endif

#define GPIO_BUFFER_SIZE 50
#define GPIO_MINDELAY 200000

typedef struct {
#ifdef __GPIO_SIMULATE
  int ifd;
#endif
  int fd;
  unsigned char number;
  char value;
  void *user_data1;
  void *user_data2;
} TGpioPort;

typedef void (*_func_gpio_portvalue)(TGpioPort *port);

// void *gpio_init(_func_gpio_portchanged on_portchanged);
// void gpio_free(void *_gpio);

void gpio_wait(TGpioPort *port, int count, int usec,
               _func_gpio_portvalue on_portvalue);
char gpio_set_value(unsigned char port_number, char value);
char gpio_get_value(unsigned char port_number, char *value);
char gpio_port_init(unsigned char port_number, unsigned char in, char value);

#ifdef __cplusplus
}
#endif

#endif /* GPIO_H_ */
