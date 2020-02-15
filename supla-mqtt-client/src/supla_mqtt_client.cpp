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

#define NO_DATABASE
#define NO_USER

int main(int argc, char *argv[]) {
  void *client_loop_t = NULL;

  supla_log(LOG_INFO, "initializing main loop");

  if (clientcfg_init(argc, argv) == 0) {
    clientcfg_free();
    return EXIT_FAILURE;
  }

  st_mainloop_init();
  st_hook_signals();

  void *sclient = NULL;
  client_loop_t = sthread_simple_run(client_loop, (void *)&sclient, 0);

  while (st_app_terminate == 0) {
    st_mainloop_wait(5000);
  }

  sthread_twf(client_loop_t);
  st_mainloop_free();
  clientcfg_free();

  return EXIT_SUCCESS;
}
