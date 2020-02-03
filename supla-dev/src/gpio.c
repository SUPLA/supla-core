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

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#ifdef __GPIO_SIMULATE
#include <sys/inotify.h>
#endif

#include "gpio.h"
#include "log.h"
#include "tools.h"

#define GPIO_BUFFER_SIZE 50

char *gpio_get_file(char *buffer, unsigned char port_number, const char *file) {
#ifdef __GPIO_SIMULATE
  snprintf(buffer, GPIO_BUFFER_SIZE, "/supla_virtual_dev/gpio/gpio%i/%s",
           port_number, file);
#else
  snprintf(buffer, GPIO_BUFFER_SIZE, "/sys/class/gpio/gpio%i/%s", port_number,
           file);
#endif
  return buffer;
}

char gpio_write_file(const char *file, const char *str) {
  int fd;

  fd = open(file, O_WRONLY);

  if (fd != -1) {
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wunused-result"
    write(fd, str, strlen(str));
    #pragma GCC diagnostic pop
    close(fd);
    return 1;
  }

  return 0;
}

char gpio_read_file(const char *file, char *buff, int buff_len) {
  int fd;
  char result = 0;
  int readed;

  fd = open(file, O_RDONLY);
  if (fd != -1) {
    if (-1 != (readed = read(fd, buff, buff_len))) {
      result = 1;
      if (buff_len > 0 && readed > 0 && buff[readed - 1] == 10)
        buff[readed - 1] = 0;
      else
        buff[readed] = 0;
    }
    close(fd);
  }

  return result;
}

void gpio_read_and_raise(TGpioPort *port, _func_gpio_portvalue on_portvalue) {
  char value[2];

  if (read(port->fd, value, 2) > 0) {
    value[0] = value[0] == 49 ? 1 : 0;
    port->value = value[0];
    on_portvalue(port);
  }
}

void gpio_wait(TGpioPort *port, int count, int usec,
               _func_gpio_portvalue on_portvalue) {
  char buffer[GPIO_BUFFER_SIZE];
  int a, max;
  struct timeval wait_tv;

#ifdef __GPIO_SIMULATE
  int inot_fd = inotify_init();
#endif

  fd_set rfds;
  FD_ZERO(&rfds);
  max = -1;

  // supla_log(LOG_DEBUG, "gpio_wait");

  wait_tv.tv_sec = (int)(usec / 1000000);
  wait_tv.tv_usec = (int)(usec % 1000000);

  for (a = 0; a < count; a++) {
    port[a].fd = open(gpio_get_file(buffer, port[a].number, "value"), O_RDONLY);

#ifdef __GPIO_SIMULATE

    port[a].ifd = inotify_add_watch(
        inot_fd, gpio_get_file(buffer, port[a].number, "value"), IN_MODIFY);

    if (port[a].fd != -1 && port[a].ifd != -1) {
      gpio_read_and_raise(&port[a], on_portvalue);
    }

#else

    if (port[a].fd != -1) {
      if (port[a].fd > max) max = port[a].fd;

      FD_SET(port[a].fd, &rfds);
      gpio_read_and_raise(&port[a], on_portvalue);
    }
#endif
  }

#ifdef __GPIO_SIMULATE
  if (inot_fd > max) max = inot_fd;

  FD_SET(inot_fd, &rfds);
#endif

  if (max != -1) {
#ifdef __GPIO_SIMULATE
    select(max + 1, &rfds, NULL, NULL, &wait_tv);
#else
    select(max + 1, NULL, NULL, &rfds, &wait_tv);
#endif
  }

  for (a = 0; a < count; a++)
    if (port[a].fd != -1) {
      gpio_read_and_raise(&port[a], on_portvalue);

#ifdef __GPIO_SIMULATE
      if (port[a].ifd != -1) inotify_rm_watch(inot_fd, port[a].ifd);
#endif

      close(port[a].fd);
      port[a].fd = -1;
    }

#ifdef __GPIO_SIMULATE
  close(inot_fd);
#endif
}

char gpio_set_value(unsigned char port_number, char value) {
  supla_log(LOG_DEBUG, "gpio_set_value port: %i, value: %i", port_number,
            value);

  char v[2];
  char buffer[GPIO_BUFFER_SIZE];

  if (port_number == 0) return 0;

  v[0] = value == 49 || value == 1 ? 49 : 48;
  v[1] = 0;
  return gpio_write_file(gpio_get_file(buffer, port_number, "value"), v);
}

char gpio_get_value(unsigned char port_number, char *value) {
  char v[2];
  char buffer[GPIO_BUFFER_SIZE];

  if (port_number == 0) return 0;

  if (1 == gpio_read_file(gpio_get_file(buffer, port_number, "value"), v, 2)) {
    *value = v[0] == 49 ? 1 : 0;
    return 1;
  }

  return 0;
}
char gpio_set_params(char *buffer, const char port_number, const char *dir_val,
                     const char *edge_val) {
  const char dir_str[] = "direction";
  const char edge_str[] = "edge";
  char rbuff[5];

  gpio_write_file(gpio_get_file(buffer, port_number, dir_str), dir_val);
  gpio_write_file(gpio_get_file(buffer, port_number, edge_str), edge_val);

  if (0 ==
      gpio_read_file(gpio_get_file(buffer, port_number, dir_str), rbuff, 5))
    return 0;

  if (strcmp(rbuff, dir_val) != 0) return 0;

  if (0 ==
      gpio_read_file(gpio_get_file(buffer, port_number, edge_str), rbuff, 5))
    return 0;

  if (strcmp(rbuff, edge_val) != 0) return 0;

  return 1;
}

#ifdef __GPIO_SIMULATE
void gpio_create_file(unsigned char port_number, char *file) {
  char buffer[GPIO_BUFFER_SIZE];
  int fd;

  fd = open(gpio_get_file(buffer, port_number, file), O_RDWR | O_CREAT, 0755);
  if (fd != -1) {
    close(fd);
  } else {
    supla_log(LOG_DEBUG, "Can't create %s", buffer);
  }
}
#endif

char gpio_port_init(unsigned char port_number, unsigned char in, char value) {
  char result = 0;

  char buffer[GPIO_BUFFER_SIZE];
  const char val_str[] = "value";

  if (st_file_exists(gpio_get_file(buffer, port_number, val_str)) == 0) {
#ifdef __GPIO_SIMULATE
    snprintf(buffer, GPIO_BUFFER_SIZE, "/supla_virtual_dev/gpio/gpio%d",
             port_number);

    if (st_file_exists(buffer) == 0) {
      mkdir(buffer, 0755);
      gpio_set_value(port_number, 0);

      gpio_create_file(port_number, "value");
      gpio_create_file(port_number, "direction");
      gpio_create_file(port_number, "edge");
    }

    snprintf(buffer, GPIO_BUFFER_SIZE, "%d", port_number);
#else
    snprintf(buffer, GPIO_BUFFER_SIZE, "%d", port_number);
    gpio_write_file("/sys/class/gpio/export", buffer);
#endif
  }

  if (st_file_exists(gpio_get_file(buffer, port_number, val_str)) == 0) {
    supla_log(LOG_ERR, "GPIO %d does not exist.", port_number);
    return 0;
  }

  if (in == 1) {
    result = gpio_set_params(buffer, port_number, "in", "both");
  } else {
    result = gpio_set_params(buffer, port_number, "out", "none");
  }

  if (result == 0)
    supla_log(LOG_ERR, "Can't set GPIO port parameters. Port number %d",
              port_number);

  return result;
}
