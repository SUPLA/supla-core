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

#include "svrcfg.h"
#include <stdlib.h>

unsigned char svrcfg_init(int argc, char *argv[]) {
  char result;
  // !!! order is important !!!

  char *s_global = "GLOBAL";
  scfg_add_str_param(s_global, "user", NULL);
  scfg_add_str_param(s_global, "group", NULL);

  char *s_net = "NET";
  scfg_add_int_param(s_net, "tcp_port", 2015);
  scfg_add_bool_param(s_net, "tcp_enabled", 1);

  scfg_add_int_param(s_net, "ssl_port", 2016);
  scfg_add_bool_param(s_net, "ssl_enabled", 1);

  scfg_add_str_param(s_net, "cert", NULL);
  scfg_add_str_param(s_net, "private_key", NULL);

  char *s_mysql = "MySQL";
  scfg_add_str_param(s_mysql, "host", NULL);
  scfg_add_int_param(s_mysql, "port", 0);
  scfg_add_str_param(s_mysql, "database", "supla");
  scfg_add_str_param(s_mysql, "user", NULL);
  scfg_add_str_param(s_mysql, "password", NULL);

  result = scfg_load(argc, argv, "/etc/supla-server/supla.cfg");
  scfg_names_free();
  return result;
}
