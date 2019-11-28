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
#include "client.h"
#include "database.h"
#include "device.h"
#include "ipcctrl.h"
#include "ipcsocket.h"
#include "log.h"
#include "safearray.h"
#include "serverconnection.h"
#include "sthread.h"
#include "supla-socket.h"
#include "svrcfg.h"
#include "tools.h"

// SERVER CONNECTION BLOCK BEGIN ---------------------------------------

void accept_loop_srvconn_execute(void *svrconn, void *sthread) {
  database::thread_init();
  ((serverconnection *)svrconn)->execute(sthread);
}

void accept_loop_srvconn_finish(void *svrconn, void *sthread) {
  delete (serverconnection *)svrconn;
  database::thread_end();
}

char accept_loop_srvconn_thread_cnd(void *svrconn_sthread) {
  if (sthread_isfinished(svrconn_sthread) == 1) {
    sthread_free(svrconn_sthread);
    return 1;
  }

  return 0;
}

char accept_loop_srvconn_thread_twt(void *svrconn_sthread) {
  sthread_twf(svrconn_sthread);
  return 1;
}

void accept_loop(void *ssd, void *al_sthread) {
  void *supla_socket = NULL;
  void *svrconn_thread_arr = safe_array_init();

  int concurrent_registrations_limit =
      scfg_int(CFG_LIMIT_CONCURRENT_REGISTRATIONS);

  struct timeval reg_limit_exceeded_alert_time = {0, 0};
  struct timeval reg_limit_exceeded_time = {0, 0};
  struct timeval now;

  while (sthread_isterminated(al_sthread) == 0 && st_app_terminate == 0) {
    safe_array_clean(svrconn_thread_arr, accept_loop_srvconn_thread_cnd);

    gettimeofday(&now, NULL);

    if (concurrent_registrations_limit > 0 &&
        serverconnection::registration_pending_count() >=
            concurrent_registrations_limit) {
      if (reg_limit_exceeded_alert_time.tv_sec == 0) {
        supla_log(LOG_ALERT, "Concurrent registration limit exceeded (%i)",
                  concurrent_registrations_limit);
        reg_limit_exceeded_alert_time = now;
      } else if (now.tv_sec - reg_limit_exceeded_alert_time.tv_sec >= 600) {
        supla_log(
            LOG_ALERT,
            "Exceeded number of concurrent registrations takes too long! (%i)",
            concurrent_registrations_limit);
        reg_limit_exceeded_alert_time = now;
      }

      reg_limit_exceeded_time = now;

    } else if (reg_limit_exceeded_time.tv_sec &&
               now.tv_sec - reg_limit_exceeded_time.tv_sec >= 10) {
      reg_limit_exceeded_time = {0, 0};
      reg_limit_exceeded_alert_time = {0, 0};
      supla_log(LOG_INFO,
                "The number of concurrent registrations returned below the "
                "limit");
    }

    unsigned int ipv4;

    if (ssocket_accept(ssd, &ipv4, &supla_socket) != 0 &&
        supla_socket != NULL) {
      Tsthread_params stp;

      stp.execute = accept_loop_srvconn_execute;
      stp.finish = accept_loop_srvconn_finish;
      stp.user_data = new serverconnection(ssd, supla_socket, ipv4);
      stp.free_on_finish = 0;
      stp.initialize = NULL;

      safe_array_add(svrconn_thread_arr, sthread_run(&stp));
    }
  }

  safe_array_clean(svrconn_thread_arr, accept_loop_srvconn_thread_twt);
  safe_array_free(svrconn_thread_arr);
}

// SERVER CONNECTION BLOCK BEGIN ---------------------------------------

// IPC CTRL BLOCK BEGIN ------------------------------------------------

void accept_loop_ipcctrl_execute(void *ipcctrl, void *sthread) {
  database::thread_init();
  ((svr_ipcctrl *)ipcctrl)->execute(sthread);
}

void accept_loop_ipcctrl_finish(void *ipcctrl, void *sthread) {
  delete (svr_ipcctrl *)ipcctrl;
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
      stp.user_data = new svr_ipcctrl(client_sd);
      stp.free_on_finish = 0;
      stp.initialize = NULL;

      safe_array_add(ipcctrl_thread_arr, sthread_run(&stp));
    }
  }

  safe_array_clean(ipcctrl_thread_arr, accept_loop_ipcctrl_thread_twt);
  safe_array_free(ipcctrl_thread_arr);
}

// IPC CTRL BLOCK END --------------------------------------------------
