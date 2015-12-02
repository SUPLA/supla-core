/*
 ============================================================================
 Name        : supla.c
 Author      : Przemyslaw Zygmunt p.zygmunt@acsoftware.pl [AC SOFTWARE]
 Version     : 1.0
 Copyright   : GPLv2
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/utsname.h>

#include "tools.h"
#include "log.h"
#include "devcfg.h"
#include "devconnection.h"
#include "channel-io.h"

int main(int argc, char* argv[]) {


	// INIT BLOCK

#ifdef __DEBUG
	char GUIDHEX[SUPLA_GUID_HEXSIZE+1];
#endif


#ifdef __SINGLE_THREAD
	supla_log(LOG_DEBUG, "SINGLE THREAD MODE");
#endif


	if ( channelio_init() != 1 ) {  // Before cfg init
		supla_log(LOG_ERR, "Channel I/O init error!");
		return EXIT_FAILURE;
	}

	if ( devcfg_init(argc, argv) == 0 ) {
		channelio_free();
		return EXIT_FAILURE;
	}

    if ( devcfg_getdev_guid() == 0 ) {
    	supla_log(LOG_ERR, "Device GUID error!");
    	goto exit_fail;
    }

	if ( run_as_daemon
		 && 0 == st_try_fork() ) {
		goto exit_fail;
	}

	supla_log(LOG_DEBUG, "Protocol v%d", SUPLA_PROTO_VERSION);
	supla_write_state_file(scfg_string(CFG_STATE_FILE), LOG_DEBUG, "Starting...");

#ifdef __DEBUG
    st_guid2hex(GUIDHEX, DEVICE_GUID);
    GUIDHEX[SUPLA_GUID_HEXSIZE] = 0;
    supla_log(LOG_DEBUG, "Device GUID: %s", GUIDHEX);
#endif

	st_setpidfile(pidfile_path);
	st_mainloop_init();
	st_hook_signals();

	channelio_channel_init();

	// CONNECTION START

	void *dconn = devconnection_start();

	// MAIN LOOP

	while(st_app_terminate == 0) {
		st_mainloop_wait(1000000);
	}

	// RELEASE BLOCK

	devconnection_stop(dconn);
	st_delpidfile(pidfile_path);
	devcfg_free();
	channelio_free();
	st_mainloop_free(); // after channelio_free

	return EXIT_SUCCESS;

	// INIT BLOCK FAIL
	exit_fail:

	devcfg_free();
	channelio_free();
	exit(EXIT_FAILURE);
}
