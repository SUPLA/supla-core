//============================================================================
// Name        : supla-console-client.cpp
// Author      : AC SOFTWARE SP. Z O.O.
// Version     : v1.0
// Copyright   : GPLv2
//============================================================================

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "supla-client-lib/supla-client.h"
#include "supla-client-lib/log.h"
#include "supla-client-lib/sthread.h"
#include "supla-client-lib/tools.h"
#include "clientcfg.h"
#include "client_loop.h"

int getch()
{
    int r;
    unsigned char c;
    if ((r = read(0, &c, sizeof(c))) < 0)
        return r;

    return c;
}

int kbhit()
{
    struct timeval tv = { 0L, 0L };
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(0, &fds);
    return select(1, &fds, NULL, NULL, &tv);
}


int main(int argc, char* argv[]) {

	void *client_loop_t = NULL;

	if ( clientcfg_init(argc, argv) == 0 ) {
		clientcfg_free();
		return EXIT_FAILURE;
	}

	st_mainloop_init();
	st_hook_signals();

	// CLIENT LOOP
	void *sclient = NULL;
	client_loop_t = sthread_simple_run(client_loop, (void*)&sclient, 0);

	// MAIN LOOP

	while(st_app_terminate == 0) {

		if ( kbhit() > 0 ) {
            switch(getch()) {
            case '0':
            	supla_client_open(sclient, 151, 0);
            	break;
            case '1':
            	supla_client_open(sclient, 151, 1);
            	break;
            case '2':
            	supla_client_open(sclient, 151, 2);
            	break;

            case '4':
            	supla_client_open(sclient, 28, 1);
            	break;
            case '5':
            	supla_client_open(sclient, 29, 1);
            	break;
            case '6':
            	supla_client_open(sclient, 30, 1);
            	break;
            }
		}

		st_mainloop_wait(10000);
	}

	// RELEASE BLOCK
	sthread_twf(client_loop_t);
	st_mainloop_free();
	clientcfg_free();

	return EXIT_SUCCESS;

}
