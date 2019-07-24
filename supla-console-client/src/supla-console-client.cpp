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

#if defined(__DEBUG) && defined(__SSOCKET_WRITE_TO_FILE)
  unlink("ssocket_write.raw");
#endif

  if (lifetime == 0) {
    lifetime = 5;
  }

  supla_log(LOG_INFO, "Lifetime: %i sec.", lifetime);

  st_mainloop_init();
  st_hook_signals();

  // CLIENT LOOP
  void *sclient = NULL;
  client_loop_t = sthread_simple_run(client_loop, (void *)&sclient, 0);

  // MAIN LOOP

  char calcfg_sent = 0;
  int calcfg_delay = 1;

  char actions[] = {
      SUPLA_THERMOSTAT_CMD_SET_MODE_AUTO,
      SUPLA_THERMOSTAT_CMD_SET_MODE_NORMAL, SUPLA_THERMOSTAT_CMD_SET_MODE_ECO,
      SUPLA_THERMOSTAT_CMD_SET_MODE_TURBO};

  while (st_app_terminate == 0) {
    struct timeval now;
    gettimeofday(&now, NULL);

    if (sclient && supla_client_registered(sclient) == 1) {
        if (now.tv_sec - runtime.tv_sec >= calcfg_delay && calcfg_sent == 0) {
          unsigned int seed = now.tv_sec + now.tv_usec;

          int cmd = actions[rand_r(&seed) % sizeof(actions)];
          char on = rand_r(&seed) % 2;
          supla_log(LOG_INFO, "CALCFG %i/%i", cmd, on);

          TCS_DeviceCalCfgRequest_B request;
          memset(&request, 0, sizeof(TCS_DeviceCalCfgRequest_B));

          request.Id = 3;
          request.Target = SUPLA_TARGET_CHANNEL;
          request.Command = cmd;
          request.DataSize = 1;
          request.Data[0] = on;

          supla_client_device_calcfg_request(sclient, &request);

          calcfg_sent = 1;
        }

        if (lifetime > 0) {
          if (now.tv_sec - runtime.tv_sec >= lifetime) {
            supla_log(LOG_INFO, "Timeout");
            break;
          }
        }
    }
    st_mainloop_wait(100);
  }

  // RELEASE BLOCK
  sthread_twf(client_loop_t);
  st_mainloop_free();
  clientcfg_free();

  return EXIT_SUCCESS;
}
