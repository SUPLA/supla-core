/*
 ============================================================================
 Name        : eh.h
 Author      : Przemyslaw Zygmunt p.zygmunt@acsoftware.pl [AC SOFTWARE]
 Version     : 1.0
 Copyright   : GPLv2
 ============================================================================
*/

#ifndef EH_H_
#define EH_H_

#include <sys/select.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {


	int nfds;

#ifdef __linux__
	int fd1;
#else
	int fd1[2];
#endif
	int fd2;
	int fd3;

	struct timeval tv;

}TEventHandler;

TEventHandler* eh_init(void);
void eh_add_fd(TEventHandler *eh, int fd);
void eh_raise_event(TEventHandler *eh);
int eh_wait(TEventHandler *eh, int usec);
void eh_free(TEventHandler *eh);

#ifdef __cplusplus
}
#endif

#endif /* EH_H_ */
