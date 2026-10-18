// SPDX-FileCopyrightText: AC SOFTWARE SP. Z O.O.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef EH_H_
#define EH_H_

#if !defined(ESP8266) && !defined(__AVR__) && !defined(_WIN32) && \
    !defined(ESP32) && !defined(ARDUINO) && !defined(SUPLA_DEVICE)
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
