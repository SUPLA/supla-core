/*
 ============================================================================
 Name        : ipcsocket.h
 Author      : Przemyslaw Zygmunt p.zygmunt@acsoftware.pl [AC SOFTWARE]
 Version     : 1.0
 Copyright   : GPLv2
 ============================================================================
*/


#ifndef IPCSOCKET_H_
#define IPCSOCKET_H_

#ifdef __cplusplus
extern "C" {
#endif

void* ipcsocket_init(const char *address);
void ipcsocket_close(void *ipc);
void ipcsocket_free(void *ipc);
int ipcsocket_accept(void *ipc);

#ifdef __cplusplus
}
#endif

#endif /* IPCSOCKET_H_ */
