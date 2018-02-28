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
#include <unistd.h>

#include "accept_loop.h"
#include "ipcctrl.h"
#include "ipcsocket.h"
#include "log.h"
#include "safearray.h"
#include "sthread.h"
#include "supla-socket.h"

void accept_loop_ipcctrl_finish(void *ipcctrl, void *sthread) {
  delete (svr_ipcctrl *)ipcctrl;
}

void accept_loop_ipcctrl_execute(void *ipcctrl, void *sthread) {
  ((svr_ipcctrl *)ipcctrl)->execute(sthread);
}

char accept_loop_ipcctrl_thread_cnd(void *ipcctrl_sthread) {
  if (sthread_isfinished(ipcctrl_sthread) == 1) {
    sthread_free(ipcctrl_sthread);
    return 1;
  }

  return 0;
}

char accept_loop_ipcctrl_thread_twt(void *ipcctrl_sthread) {
  sthread_twf(ipcctrl_sthread);
  return 1;
}

void ipc_accept_loop(void *ipc, void *ipc_al_sthread) {
  int client_sd;
  void *ipcctrl_thread_arr = safe_array_init();

  while (sthread_isterminated(ipc_al_sthread) == 0) {
    safe_array_clean(ipcctrl_thread_arr, accept_loop_ipcctrl_thread_cnd);

    if (-1 == (client_sd = ipcsocket_accept(ipc))) {
      break;
    } else {
      Tsthread_params stp;

      stp.execute = accept_loop_ipcctrl_execute;
      stp.finish = accept_loop_ipcctrl_finish;
      stp.user_data = new svr_ipcctrl(client_sd);
      stp.free_on_finish = 0;
      stp.initialize = NULL;

      safe_array_add(ipcctrl_thread_arr, sthread_run(&stp));
    }
  }

  safe_array_clean(ipcctrl_thread_arr, accept_loop_ipcctrl_thread_twt);
  safe_array_free(ipcctrl_thread_arr);
}
