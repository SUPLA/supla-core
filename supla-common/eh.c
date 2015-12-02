/*
 ============================================================================
 Name        : eh.c
 Author      : Przemyslaw Zygmunt p.zygmunt@acsoftware.pl [AC SOFTWARE]
 Version     : 1.0
 Copyright   : GPLv2
 ============================================================================
 */

#include "eh.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>

#ifdef __linux__
#include <sys/eventfd.h>
#include <sys/epoll.h>
#endif


TEventHandler* eh_init(void) {

    #ifdef __linux__
	struct epoll_event evnt = {0};
    #endif

	TEventHandler *eh = malloc(sizeof(TEventHandler));
	if ( eh != 0 ) {

		memset(eh, 0, sizeof(TEventHandler));

		eh->fd2 = -1;
		eh->fd3 = -1;

        #ifdef __linux__

		#ifdef __ANDROID__
		eh->epoll_fd = epoll_create(1);
		#else
		eh->epoll_fd = epoll_create1(0);
		#endif

		eh->fd1 = eventfd(0, EFD_NONBLOCK);
		eh->nfds = eh->fd1+1;


		if ( eh->epoll_fd != -1 ) {

			evnt.data.fd = eh->fd1;
			evnt.events = EPOLLIN | EPOLLET;

			if ( epoll_ctl(eh->epoll_fd, EPOLL_CTL_ADD, eh->fd1, &evnt) == -1 ) {

				close(eh->epoll_fd);
				eh->epoll_fd = -1;
			}
		}


        #else
		pipe(eh->fd1);

		if ( eh->fd1[0] != -1 )
			fcntl(eh->fd1[0], F_SETFL, O_NONBLOCK);

		if ( eh->fd1[1] != -1 )
			fcntl(eh->fd1[1], F_SETFL, O_NONBLOCK);

		eh->nfds = eh->fd1[0]+1;
        #endif

	}

    return eh;

}

void eh_add_fd(TEventHandler *eh, int fd) {

	#ifdef __linux__
    struct epoll_event evnt = {0};
    #endif

	if ( eh == 0 )
		return;

	if ( fd != -1 ) {

		if ( eh->fd2 == -1 )
			eh->fd2 = fd;
		else if ( eh->fd3 == -1 )
			eh->fd3 = fd;
		else
			fd = -1;

		if ( fd != -1 ) {

            #ifdef __linux__
			if ( eh->epoll_fd != -1 ) {

				evnt.events = EPOLLIN;
				evnt.data.fd = fd;

				if (epoll_ctl(eh->epoll_fd, EPOLL_CTL_ADD, fd, &evnt) == -1) {

					close(eh->epoll_fd);
					eh->epoll_fd = -1;
				}

			}
            #endif

		    if ( fd+1 > eh->nfds )
		    	eh->nfds = fd+1;
		}

	}

}

void eh_raise_event(TEventHandler *eh) {

	if ( eh == 0 )
		return;

    #ifdef __linux__

	uint64_t u=1;

	if ( eh->fd1 != -1 )
	  	write(eh->fd1, &u, sizeof(uint64_t));
	#else

	char u=1;

	if ( eh->fd1[1] != -1 )
	  	write(eh->fd1[1], &u, sizeof(char));
	#endif

}


int eh_wait(TEventHandler *eh, int usec) {

	int result;

	#ifdef __linux__
	uint64_t u;
	struct epoll_event events[10];
    #else
	char u;
    #endif

	if ( eh == 0 ) {
		usleep(usec);
		return -1;
	}


	assert(eh->fd3 == -1);

	eh->tv.tv_sec = (int)(usec/1000000);
	eh->tv.tv_usec = (int)(usec%1000000);

	fd_set rfds;
	FD_ZERO(&rfds);

	#ifdef __linux__
	if ( eh->fd1 != -1 )
		FD_SET(eh->fd1, &rfds);
	#else
	if ( eh->fd1[0] != -1 )
		FD_SET(eh->fd1[0], &rfds);
	#endif

	if ( eh->fd2 != -1 )
		FD_SET(eh->fd2, &rfds);

	if ( eh->fd3 != -1 )
		FD_SET(eh->fd3, &rfds);

	if ( eh->nfds > 0 ) {


        #ifdef __linux__

		if ( eh->epoll_fd == -1 ) {
			result = select(eh->nfds, &rfds, NULL, NULL, &eh->tv);
		} else {
			result = epoll_wait(eh->epoll_fd, events, 10, usec/1000);
		}

        #else
		result = select(eh->nfds, &rfds, NULL, NULL, &eh->tv);
        #endif

		if ( result > 0
            #ifdef __linux__
			 && eh->fd1 != -1 ) {
			read(eh->fd1, &u, sizeof(uint64_t));
            #else
			 && eh->fd1[0] != -1 ) {
			read(eh->fd1[0], &u, sizeof(char));
            #endif
		}

		return result;
	};

	return -1;
}

void eh_free(TEventHandler *eh) {

	if ( eh != 0 ) {

        #ifdef __linux__

		if ( eh->fd1 != -1 )
			close(eh->fd1);

		if ( eh->epoll_fd != -1 )
		    close(eh->epoll_fd);

        #else

		if ( eh->fd1[0] != -1 )
			close(eh->fd1[0]);

		if ( eh->fd1[1] != -1 )
			close(eh->fd1[1]);

        #endif

		free(eh);
	}

}
