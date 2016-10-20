/*
 ============================================================================
 Name        : eh.h
 Author      : Przemyslaw Zygmunt przemek@supla.org
 Copyright   : 2015-2016 GPLv2
 ============================================================================
*/

#ifndef EH_H_
#define EH_H_

#if !defined(ESP8266) && !defined(__AVR__) && !defined(_WIN32)
#include <sys/select.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {

	int nfds;

	#ifndef _WIN32

		#ifdef __linux__
		int epoll_fd;
		int fd1;
		#else
		int fd1[2];
		#endif

		int fd2;
		int fd3;

		struct timeval tv;

	#endif

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
