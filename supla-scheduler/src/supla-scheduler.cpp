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

#include <pthread.h>  // NOLINT
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "database.h"
#include "ipcclient.h"
#include "log.h"
#include "queue.h"
#include "schedulercfg.h"
#include "sthread.h"
#include "tools.h"

int main(int argc, char *argv[]) {
  void *queue_loop_t = NULL;

  // INIT BLOCK
  if (schedulercfg_init(argc, argv) == 0) return EXIT_FAILURE;

  {
    char dt[64];

    supla_log(LOG_INFO, "Scheduler version %s", SCHEDULER_VERSION);
    supla_log(LOG_INFO, "Started at %s", st_get_datetime_str(dt));
  }

  if (run_as_daemon && 0 == st_try_fork()) {
    goto exit_fail;
  }

  if (database::mainthread_init() == false) {
    goto exit_fail;
  }

  {
    database *db = new database();
    if (!db->check_db_version("20180814155501")) {
      delete db;
      database::mainthread_end();
      goto exit_fail;
    } else {
      delete db;
    }
  }

  if (0 == st_set_ug_id(scfg_getuid(CFG_UID), scfg_getgid(CFG_GID))) {
    goto exit_fail;
  }


  st_setpidfile(pidfile_path);
  st_mainloop_init();
  st_hook_signals();

  // INIT QUEUE LOOP
  queue_loop_t = sthread_simple_run(queue_loop, NULL, 0);

  // MAIN LOOP

  while (st_app_terminate == 0) {
    st_mainloop_wait(1000000);
  }

  // RELEASE BLOCK
  supla_log(LOG_INFO, "Shutting down...");

  sthread_twf(queue_loop_t);

  st_mainloop_free();
  st_delpidfile(pidfile_path);

  database::mainthread_end();

  scfg_free();

  {
    char dt[64];
    supla_log(LOG_INFO, "Stopped at %s", st_get_datetime_str(dt));
  }

  return EXIT_SUCCESS;

exit_fail:

  scfg_free();
  exit(EXIT_FAILURE);
}
