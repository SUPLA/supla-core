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

#include "schedulercfg.h"
#include <stdlib.h>

unsigned char schedulercfg_init(int argc, char *argv[]) {
  char result;
  // !!! order is important !!!

  char *s_global = "GLOBAL";
  scfg_add_str_param(s_global, "user", NULL);
  scfg_add_str_param(s_global, "group", NULL);

  char *s_mysql = "MySQL";
  scfg_add_str_param(s_mysql, "host", NULL);
  scfg_add_int_param(s_mysql, "port", 0);
  scfg_add_str_param(s_mysql, "database", "supla");
  scfg_add_str_param(s_mysql, "user", NULL);
  scfg_add_str_param(s_mysql, "password", NULL);

  char *s_scheduler = "SCHEDULER";
  scfg_add_int_param(s_scheduler, "max_workers", 30);
  scfg_add_int_param(s_scheduler, "max_job_per_second", 50);

  char *s_ipc = "IPC";
  scfg_add_str_param(s_ipc, "socket_path", "/tmp/supla-server-ctrl.sock");

  result = scfg_load(argc, argv, "/etc/supla-server/supla.cfg");
  scfg_names_free();
  return result;
}
