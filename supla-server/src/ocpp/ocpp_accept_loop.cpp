// SPDX-FileCopyrightText: AC SOFTWARE SP. Z O.O.
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ocpp/ocpp_accept_loop.h"

#include "ipc/ipcsocket.h"
#include "ocpp/ocpp_gateway.h"
#include "safearray.h"
#include "sthread.h"
#include "tools.h"

namespace {

void connection_execute(void *connection, void *sthread) {
  (*static_cast<std::shared_ptr<supla_ocpp_connection> *>(connection))
      ->execute(sthread);
}

void connection_finish(void *connection, void *sthread) {
  delete static_cast<std::shared_ptr<supla_ocpp_connection> *>(connection);
}

char clean_finished_thread(void *connection_thread) {
  if (sthread_isfinished(connection_thread) == 1) {
    sthread_free(connection_thread);
    return 1;
  }
  return 0;
}

char terminate_thread(void *connection_thread) {
  sthread_twf(connection_thread, true);
  return 1;
}

}  // namespace

void ocpp_accept_loop(void *ipc, void *accept_loop_thread) {
  void *connection_threads = safe_array_init();

  while (sthread_isterminated(accept_loop_thread) == 0 &&
         st_app_terminate == 0) {
    safe_array_clean(connection_threads, clean_finished_thread);

    int client_socket = ipcsocket_accept(ipc);
    if (client_socket == -1) {
      break;
    }

    Tsthread_params params = {};
    params.execute = connection_execute;
    params.finish = connection_finish;
    params.user_data = new std::shared_ptr<supla_ocpp_connection>(
        std::make_shared<supla_ocpp_connection>(client_socket));
    params.free_on_finish = 0;

    void *connection_thread = nullptr;
    sthread_run(&params, &connection_thread);
    safe_array_add(connection_threads, connection_thread);
  }

  safe_array_clean(connection_threads, terminate_thread);
  safe_array_free(connection_threads);
}
