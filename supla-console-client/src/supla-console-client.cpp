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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "client_loop.h"
#include "clientcfg.h"
#include "supla-client-lib/log.h"
#include "supla-client-lib/sthread.h"
#include "supla-client-lib/supla-client.h"
#include "supla-client-lib/tools.h"

int getch() {
  int r;
  unsigned char c;
  if ((r = read(0, &c, sizeof(c))) < 0) return r;

  return c;
}

int kbhit() {
  struct timeval tv = {0L, 0L};
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(0, &fds);
  return select(1, &fds, NULL, NULL, &tv);
}

int main(int argc, char *argv[]) {
  void *client_loop_t = NULL;

  if (clientcfg_init(argc, argv) == 0) {
    clientcfg_free();
    return EXIT_FAILURE;
  }

  struct timeval runtime;
  gettimeofday(&runtime, NULL);

  if (lifetime > 0) {
    supla_log(LOG_INFO, "Lifetime: %i sec.", lifetime);
  }

  if (input_off == 1) {
    supla_log(LOG_INFO, "Input: off");
  }

  st_mainloop_init();
  st_hook_signals();

  // CLIENT LOOP
  void *sclient = NULL;
  client_loop_t = sthread_simple_run(client_loop, (void *)&sclient, 0);

  // MAIN LOOP

  while (st_app_terminate == 0) {
    if (input_off == 0 && sclient != NULL && kbhit() > 0) {
      switch (getch()) {
        case '0':
          supla_client_open(sclient, 151, 0);
          break;
        case '1':
          supla_client_open(sclient, 151, 1);
          break;
        case '2':
          supla_client_open(sclient, 151, 2);
          break;

        case '4':
          supla_client_open(sclient, 28, 1);
          break;
        case '5':
          supla_client_open(sclient, 29, 1);
          break;
        case '6':
          supla_client_open(sclient, 30, 1);
          break;
        case '7':
          supla_client_get_registration_enabled(sclient);
          break;
      }
    }

    if (lifetime > 0) {
      struct timeval now;
      gettimeofday(&now, NULL);

      if (now.tv_sec - runtime.tv_sec >= lifetime) {
        supla_log(LOG_INFO, "Timeout");
        break;
      }
    }

    st_mainloop_wait(1000);
  }

  // RELEASE BLOCK
  sthread_twf(client_loop_t);
  st_mainloop_free();
  clientcfg_free();

  return EXIT_SUCCESS;
}
