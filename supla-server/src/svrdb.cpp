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

#include "svrdb.h"
#include "svrcfg.h"

svrdb::svrdb(void) : dbcommon() {}

svrdb::~svrdb(void) {}

char *svrdb::cfg_get_host(void) { return scfg_string(CFG_MYSQL_HOST); }

char *svrdb::cfg_get_user(void) { return scfg_string(CFG_MYSQL_USER); }

char *svrdb::cfg_get_password(void) { return scfg_string(CFG_MYSQL_PASSWORD); }

char *svrdb::cfg_get_database(void) { return scfg_string(CFG_MYSQL_DB); }

int svrdb::cfg_get_port(void) { return scfg_int(CFG_MYSQL_PORT); }
