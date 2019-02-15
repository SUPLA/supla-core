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

#ifndef suplasvrcfg_H_
#define suplasvrcfg_H_

#include "cfg.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SCHEDULER_VERSION "2.3.1"

#define CFG_UID 0
#define CFG_GID 1

#define CFG_SCHEDULER_MYSQL_HOST 2
#define CFG_SCHEDULER_MYSQL_PORT 3
#define CFG_SCHEDULER_MYSQL_DB 4
#define CFG_SCHEDULER_MYSQL_USER 5
#define CFG_SCHEDULER_MYSQL_PASSWORD 6

#define CFG_MYSQL_HOST 7
#define CFG_MYSQL_PORT 8
#define CFG_MYSQL_DB 9
#define CFG_MYSQL_USER 10
#define CFG_MYSQL_PASSWORD 11

#define CFG_MAX_WORKERS 12
#define CFG_MAX_JOB_PER_SECOND 13

#define CFG_IPC_SOCKET_PATH 14

unsigned char schedulercfg_init(int argc, char* argv[]);

#ifdef __cplusplus
}
#endif

#endif /* suplasvrcfg_H_ */
