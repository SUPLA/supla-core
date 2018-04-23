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

#ifndef supladevcfg_H_
#define supladevcfg_H_

#include "cfg.h"
#include "proto.h"

extern char DEVICE_GUID[SUPLA_GUID_SIZE];
extern char DEVICE_AUTHKEY[SUPLA_AUTHKEY_SIZE];

#ifdef __cplusplus
extern "C" {
#endif

#define CFG_GUID_FILE 0
#define CFG_ALTCFG_FILE 1
#define CFG_STATE_FILE 2
#define CFG_DEVNAME 3
#define CFG_SERVER_HOST 4
#define CFG_SERVER_TCPPORT 5
#define CFG_SERVER_SSLPORT 6
#define CFG_SERVER_SSLENABLED 7
#define CFG_PROTO 8
#define CFG_LOCATION_ID 9
#define CFG_LOCATION_PWD 10
#define CFG_EMAIL 11

unsigned char devcfg_init(int argc, char* argv[]);
char devcfg_getdev_guid();
char devcfg_getdev_authkey();
void devcfg_free(void);

#ifdef __cplusplus
}
#endif

#endif /* supladevcfg_H_ */
