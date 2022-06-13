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

#ifndef ARDUINO
#include "eh.h"
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include <assert.h>
#include <fcntl.h>

#ifdef __linux__
#include <sys/epoll.h>
#include <sys/eventfd.h>
#endif

TEventHandler *eh_init(void) {
#ifdef __linux__
  struct epoll_event evnt = {0};
#endif

#ifdef _WIN32
  return 0;
#else

  TEventHandler *eh = malloc(sizeof(TEventHandler));
  if (eh != 0) {
    memset(eh, 0, sizeof(TEventHandler));

    eh->fd2 = -1;
    eh->fd3 = -1;

#ifdef __linux__

#ifdef __ANDROID__
    eh->epoll_fd = epoll_create(1);
#else
    eh->epoll_fd = epoll_create1(0);
#endif

    eh->fd1 = eventfd(0, EFD_NONBLOCK);
    eh->nfds = eh->fd1 + 1;

    if (eh->epoll_fd != -1) {
      evnt.data.fd = eh->fd1;
      evnt.events = EPOLLIN | EPOLLET;

      if (epoll_ctl(eh->epoll_fd, EPOLL_CTL_ADD, eh->fd1, &evnt) == -1) {
        close(eh->epoll_fd);
        eh->epoll_fd = -1;
      }
    }

#else
    pipe(eh->fd1);

    if (eh->fd1[0] != -1) fcntl(eh->fd1[0], F_SETFL, O_NONBLOCK);

    if (eh->fd1[1] != -1) fcntl(eh->fd1[1], F_SETFL, O_NONBLOCK);

    eh->nfds = eh->fd1[0] + 1;
#endif
  }

  return eh;
#endif
}

void eh_add_fd(TEventHandler *eh, int fd) {
#ifdef __linux__
  struct epoll_event evnt = {0};
#endif

  if (eh == 0) return;

#ifndef _WIN32
  if (fd != -1) {
    if (eh->fd2 == -1)
      eh->fd2 = fd;
    else if (eh->fd3 == -1)
      eh->fd3 = fd;
    else
      fd = -1;

    if (fd != -1) {
#ifdef __linux__
      if (eh->epoll_fd != -1) {
        evnt.events = EPOLLIN;
        evnt.data.fd = fd;

        if (epoll_ctl(eh->epoll_fd, EPOLL_CTL_ADD, fd, &evnt) == -1) {
          close(eh->epoll_fd);
          eh->epoll_fd = -1;
        }
      }
#endif

      if (fd + 1 > eh->nfds) eh->nfds = fd + 1;
    }
  }
#endif
}

void eh_raise_event(TEventHandler *eh) {
  if (eh == 0) return;

#ifdef __linux__

  uint64_t u = 1;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
  if (eh->fd1 != -1) write(eh->fd1, &u, sizeof(uint64_t));
#pragma GCC diagnostic pop

#elif !defined(_WIN32)

  char u = 1;

  if (eh->fd1[1] != -1) write(eh->fd1[1], &u, sizeof(char));
#endif
}

int eh_wait(TEventHandler *eh, int usec) {
#ifdef _WIN32
  SleepEx(usec, TRUE);  // usec mean msec
  return -1;
#else
  int result;

#ifdef __linux__
  uint64_t u;
  struct epoll_event events[10];
#else
  char u;
#endif

  if (eh == 0) {
    usleep(usec);
    return -1;
  }

  assert(eh->fd3 == -1);

  eh->tv.tv_sec = (int)(usec / 1000000);
  eh->tv.tv_usec = (int)(usec % 1000000);

  fd_set rfds;
  FD_ZERO(&rfds);

#ifdef __linux__
  if (eh->epoll_fd == -1) {
    if (eh->fd1 != -1) {
      FD_SET(eh->fd1, &rfds);
    }

    if (eh->fd2 != -1) {
      FD_SET(eh->fd2, &rfds);
    }

    if (eh->fd3 != -1) {
      FD_SET(eh->fd3, &rfds);
    }
  }
#else

  if (eh->fd1[0] != -1) {
    FD_SET(eh->fd1[0], &rfds);
  }

  if (eh->fd2 != -1) {
    FD_SET(eh->fd2, &rfds);
  }

  if (eh->fd3 != -1) {
    FD_SET(eh->fd3, &rfds);
  }

#endif /*__linux__*/

  if (eh->nfds > 0) {
#ifdef __linux__

    if (eh->epoll_fd == -1) {
      result = select(eh->nfds, &rfds, NULL, NULL, &eh->tv);
    } else {
      result = epoll_wait(eh->epoll_fd, events, 10, usec / 1000);
    }

#else
    result = select(eh->nfds, &rfds, NULL, NULL, &eh->tv);
#endif

    if (result > 0
#ifdef __linux__
        && eh->fd1 != -1) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
      read(eh->fd1, &u, sizeof(uint64_t));
#pragma GCC diagnostic pop
#else
        && eh->fd1[0] != -1) {
      read(eh->fd1[0], &u, sizeof(char));
#endif
    }

    return result;
  };

  return -1;
#endif
}

void eh_free(TEventHandler *eh) {
#ifndef _WIN32
  if (eh != 0) {
#ifdef __linux__

    if (eh->fd1 != -1) close(eh->fd1);

    if (eh->epoll_fd != -1) close(eh->epoll_fd);

#else

    if (eh->fd1[0] != -1) close(eh->fd1[0]);

    if (eh->fd1[1] != -1) close(eh->fd1[1]);

#endif

    free(eh);
  }
#endif
}
#endif /* ARDUINO */
