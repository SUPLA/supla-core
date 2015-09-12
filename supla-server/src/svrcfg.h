/*
 ============================================================================
 Name        : svrcfg.h
 Author      : Przemyslaw Zygmunt p.zygmunt@acsoftware.pl [AC SOFTWARE]
 Version     : 1.0
 Copyright   : GPLv2
 ============================================================================
 */

#ifndef suplasvrcfg_H_
#define suplasvrcfg_H_

#include "cfg.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SERVER_VERSION "1.0"

#define CFG_UID           0
#define CFG_GID           1

#define CFG_TCP_PORT      2
#define CFG_TCP_ENABLED   3

#define CFG_SSL_PORT      4
#define CFG_SSL_ENABLED   5
#define CFG_SSL_CERT      6
#define CFG_SSL_KEY       7

#define CFG_MYSQL_HOST      8
#define CFG_MYSQL_PORT      9
#define CFG_MYSQL_DB        10
#define CFG_MYSQL_USER      11
#define CFG_MYSQL_PASSWORD  12

unsigned char svrcfg_init(int argc, char* argv[]);


#ifdef __cplusplus
}
#endif

#endif /* suplasvrcfg_H_ */
