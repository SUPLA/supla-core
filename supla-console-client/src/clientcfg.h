/*
 ============================================================================
 Name        : clientcfg.h
 Author      : Przemyslaw Zygmunt p.zygmunt@acsoftware.pl [AC SOFTWARE]
 Version     : 1.0
 Copyright   : GPLv2
 ============================================================================
 */

#ifndef CLIENTCFG_H_
#define CLIENTCFG_H_

#include "supla-client-lib/proto.h"

#ifdef __cplusplus
extern "C" {
#endif

extern char *cfg_id_file;

extern char *cfg_host;
extern int cfg_port;
extern char cfg_ssl_enabled;

extern int cfg_aid;
extern char *cfg_pwd;

extern char cfg_client_GUID[SUPLA_GUID_SIZE];

unsigned char clientcfg_init(int argc, char* argv[]);
void clientcfg_free();

#ifdef __cplusplus
}
#endif

#endif /* CLIENTCFG_H_ */
