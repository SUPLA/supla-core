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
#include <sys/types.h>
#include <sys/utsname.h>
#include <unistd.h>

#include "accept_loop.h"
#include "channel-io.h"
#include "devcfg.h"
#include "devconnection.h"
#include "ipcsocket.h"
#include "log.h"
#include "sthread.h"
#include "tools.h"

void exit_fail() {
  devcfg_free();
  channelio_free();
  exit(EXIT_FAILURE);
}

int main(int argc, char* argv[]) {
// INIT BLOCK
#ifndef __SINGLE_THREAD
  void* ipc_accept_loop_t = NULL;
  void* ipc = NULL;
#endif

#ifdef __DEBUG
  char GUIDHEX[SUPLA_GUID_HEXSIZE + 1];
#endif

#ifdef __SINGLE_THREAD
  supla_log(LOG_DEBUG, "SINGLE THREAD MODE");
#endif

  if (channelio_init() != 1) {  // Before cfg init
    supla_log(LOG_ERR, "Channel I/O init error!");
    return EXIT_FAILURE;
  }

  if (devcfg_init(argc, argv) == 0) {
    channelio_free();
    return EXIT_FAILURE;
  }

  if (devcfg_getdev_guid() == 0) {
    supla_log(LOG_ERR, "Device GUID error!");
    exit_fail();
  }

  if (devcfg_getdev_authkey() == 0) {
    supla_log(LOG_ERR, "AuthKey error!");
    exit_fail();
  }

  if (run_as_daemon && 0 == st_try_fork()) {
    exit_fail();
  }

  supla_log(LOG_DEBUG, "Protocol v%d", SUPLA_PROTO_VERSION);
  supla_write_state_file(scfg_string(CFG_STATE_FILE), LOG_DEBUG, "Starting...");

#ifdef __DEBUG
  st_guid2hex(GUIDHEX, DEVICE_GUID);
  GUIDHEX[SUPLA_GUID_HEXSIZE] = 0;
  supla_log(LOG_DEBUG, "Device GUID: %s", GUIDHEX);
#endif

  st_setpidfile(pidfile_path);
  st_mainloop_init();
  st_hook_signals();

#ifndef __SINGLE_THREAD
  ipc = ipcsocket_init("/tmp/supla-dev-ctrl.sock");
#endif

  channelio_channel_init();

  // CONNECTION START

  void* dconn = devconnection_start();

// ACCEPT LOOP

#ifndef __SINGLE_THREAD
  if (ipc) ipc_accept_loop_t = sthread_simple_run(ipc_accept_loop, ipc, 0);
#endif

  // MAIN LOOP

  while (st_app_terminate == 0) {
    st_mainloop_wait(1000000);
  }

// RELEASE BLOCK

#ifndef __SINGLE_THREAD
  if (ipc != NULL) {
    ipcsocket_close(ipc);
    sthread_twf(ipc_accept_loop_t);  // ! after ipcsocket_close and before
                                     // ipcsocket_free !
    ipcsocket_free(ipc);
  }
#endif

  devconnection_stop(dconn);
  st_delpidfile(pidfile_path);
  devcfg_free();
  channelio_free();
  st_mainloop_free();  // after channelio_free

  return EXIT_SUCCESS;
}
