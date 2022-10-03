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

#include "accept_loop.h"

#include <stdio.h>
#include <unistd.h>

#include "client.h"
#include "conn/connection.h"
#include "db/database.h"
#include "device.h"
#include "ipc/ipc_ctrl.h"
#include "ipc/ipc_socket_adapter.h"
#include "ipc/ipcsocket.h"
#include "log.h"
#include "safearray.h"
#include "sthread.h"
#include "supla-socket.h"
#include "svrcfg.h"
#include "tools.h"

// SERVER CONNECTION BLOCK BEGIN ---------------------------------------

void accept_loop_connection_execute(void *connection, void *sthread) {
  database::thread_init();
  ((supla_connection *)connection)->execute(sthread);
}

void accept_loop_connection_finish(void *connection, void *sthread) {
  delete (supla_connection *)connection;
  database::thread_end();
}

char accept_loop_connection_thread_cnd(void *connection_sthread) {
  if (sthread_isfinished(connection_sthread) == 1) {
    sthread_free(connection_sthread);
    return 1;
  }

  return 0;
}

char accept_loop_connection_thread_twt(void *connection_sthread) {
  sthread_twf(connection_sthread);
  return 1;
}

void accept_loop(void *ssd, void *al_sthread) {
  void *connection_thread_arr = safe_array_init();

  while (sthread_isterminated(al_sthread) == 0 && st_app_terminate == 0) {
    safe_array_clean(connection_thread_arr, accept_loop_connection_thread_cnd);

    unsigned int ipv4;
    void *supla_socket = NULL;

    if (ssocket_accept(ssd, &ipv4, &supla_socket) != 0 &&
        supla_socket != NULL) {
      if (supla_connection::is_connection_allowed(ipv4)) {
        supla_connection *conn = new supla_connection(ssd, supla_socket, ipv4);
        if (ssocket_accept_ssl(ssd, supla_socket) == 1) {
          Tsthread_params stp;

          stp.execute = accept_loop_connection_execute;
          stp.finish = accept_loop_connection_finish;
          stp.user_data = conn;
          stp.free_on_finish = 0;
          stp.initialize = NULL;

          void *sthread = nullptr;
          sthread_run(&stp, &sthread);

          safe_array_add(connection_thread_arr, sthread);
        } else {
          delete conn;
        }
      } else {
        ssocket_supla_socket_free(supla_socket);
        supla_log(LOG_DEBUG, "Connection Dropped");
      }
    }
  }

  safe_array_clean(connection_thread_arr, accept_loop_connection_thread_twt);
  safe_array_free(connection_thread_arr);
}

// SERVER CONNECTION BLOCK BEGIN ---------------------------------------

// IPC CTRL BLOCK BEGIN ------------------------------------------------

void accept_loop_ipcctrl_execute(void *ipcctrl, void *sthread) {
  database::thread_init();
  ((supla_ipc_ctrl *)ipcctrl)->execute(sthread);
}

void accept_loop_ipcctrl_finish(void *ipcctrl, void *sthread) {
  delete (supla_ipc_ctrl *)ipcctrl;
  database::thread_end();
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

  while (sthread_isterminated(ipc_al_sthread) == 0 && st_app_terminate == 0) {
    safe_array_clean(ipcctrl_thread_arr, accept_loop_ipcctrl_thread_cnd);

    if (-1 == (client_sd = ipcsocket_accept(ipc))) {
      break;
    } else {
      Tsthread_params stp;

      stp.execute = accept_loop_ipcctrl_execute;
      stp.finish = accept_loop_ipcctrl_finish;
      stp.user_data =
          new supla_ipc_ctrl(new supla_ipc_socket_adapter(client_sd));
      stp.free_on_finish = 0;
      stp.initialize = NULL;

      void *sthread = nullptr;
      sthread_run(&stp, &sthread);

      safe_array_add(ipcctrl_thread_arr, sthread);
    }
  }

  safe_array_clean(ipcctrl_thread_arr, accept_loop_ipcctrl_thread_twt);
  safe_array_free(ipcctrl_thread_arr);
}

// IPC CTRL BLOCK END --------------------------------------------------
