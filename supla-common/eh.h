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
#ifndef EH_H_
#define EH_H_

#if !defined(ESP8266) && !defined(__AVR__) && !defined(_WIN32) && \
    !defined(ESP32)
#include <sys/select.h>
#endif

#if !defined(__AVR__) && !defined(_WIN32)
#include <sys/time.h>
#endif

#ifdef __AVR__
#include "proto.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  int nfds;

#ifndef _WIN32

#ifdef __linux__
  int epoll_fd;
  int fd1;
#else
  int fd1[2];
#endif

  int fd2;
  int fd3;

  struct timeval tv;

#endif
} TEventHandler;

TEventHandler *eh_init(void);
void eh_add_fd(TEventHandler *eh, int fd);
void eh_raise_event(TEventHandler *eh);
int eh_wait(TEventHandler *eh, int usec);
void eh_free(TEventHandler *eh);

#ifdef __cplusplus
}
#endif

#endif /* EH_H_ */
