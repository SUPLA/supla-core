/*
 ============================================================================
 Name        : devcfg.h
 Author      : Przemyslaw Zygmunt p.zygmunt@acsoftware.pl [AC SOFTWARE]
 Version     : 1.0
 Copyright   : GPLv2
 ============================================================================
 */

#ifndef supladevcfg_H_
#define supladevcfg_H_

#include "cfg.h"
#include "proto.h"

extern char DEVICE_GUID[SUPLA_GUID_SIZE];

#ifdef __cplusplus
extern "C" {
#endif

#define CFG_GUID_FILE         0
#define CFG_ALTCFG_FILE       1
#define CFG_STATE_FILE        2
#define CFG_DEVNAME           3
#define CFG_SERVER_HOST       4
#define CFG_SERVER_TCPPORT    5
#define CFG_SERVER_SSLPORT    6
#define CFG_SERVER_SSLENABLED 7
#define CFG_LOCATION_ID       8
#define CFG_LOCATION_PWD      9

unsigned char devcfg_init(int argc, char* argv[]);
char devcfg_getdev_guid();
void devcfg_free(void);

#ifdef __cplusplus
}
#endif


#endif /* supladevcfg_H_ */
