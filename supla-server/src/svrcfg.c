/*
 ============================================================================
 Name        : svrcfg.c
 Author      : Przemyslaw Zygmunt p.zygmunt@acsoftware.pl [AC SOFTWARE]
 Version     : 1.0
 Copyright   : GPLv2
 ============================================================================
 */

#include <stdlib.h>
#include "svrcfg.h"

unsigned char svrcfg_init(int argc, char* argv[]) {

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

	result =  scfg_load(argc, argv, "/etc/supla-server/supla.cfg");
	scfg_names_free();
	return result;

}
