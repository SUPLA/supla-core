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

int main(int argc, char *argv[]) {
  void *client_loop_t = NULL;

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
