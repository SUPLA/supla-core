/*
 ============================================================================
 Name        : supla-server.c
 Author      : Przemyslaw Zygmunt p.zygmunt@acsoftware.pl [AC SOFTWARE]
 Version     : 1.2
 Copyright   : GPLv2
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "supla-socket.h"
#include "ipcsocket.h"
#include "srpc.h"
#include "svrcfg.h"
#include "log.h"
#include "tools.h"
#include "proto.h"
#include "sthread.h"
#include "accept_loop.h"
#include "db.h"
#include "user.h"
#include "datalogger.h"


int main(int argc, char* argv[]) {


	void *ssd_ssl = NULL;
	void *ssd_tcp = NULL;
	void* ipc = NULL;
	void *tcp_accept_loop_t = NULL;
	void *ssl_accept_loop_t = NULL;
	void *ipc_accept_loop_t = NULL;
	void *datalogger_loop_t = NULL;

	//INIT BLOCK
	supla_log(LOG_DEBUG, "Version 1.5 [Protocol v%i]", SUPLA_PROTO_VERSION);

	if ( svrcfg_init(argc, argv) == 0 )
		return EXIT_FAILURE;

	if ( run_as_daemon
		 && 0 == st_try_fork() ) {
		goto exit_fail;
	}

	if ( database::mainthread_init() == false  ) {
		goto exit_fail;
	}


    #ifdef __OPEN_SSL
	if ( scfg_bool(CFG_SSL_ENABLED) == 1 ) {

		if ( 0 == ( ssd_ssl = ssocket_server_init(scfg_string(CFG_SSL_CERT),
                scfg_string(CFG_SSL_KEY),
                scfg_int(CFG_SSL_PORT),
                1) )
			 || 0 == ssocket_openlistener(ssd_ssl) ) {
			goto exit_fail;
		}

	}
    #endif

	if ( scfg_bool(CFG_TCP_ENABLED) == 1 ) {

		if ( 0 == ( ssd_tcp = ssocket_server_init("", "",
                scfg_int(CFG_TCP_PORT), 0) )
			 || 0 == ssocket_openlistener(ssd_tcp) ) {
			goto exit_fail;
		}

	}

	if ( 0 == st_set_ug_id(scfg_getuid(CFG_UID), scfg_getgid(CFG_GID)) ) {
		goto exit_fail;
	}


	supla_user::init();

	st_setpidfile(pidfile_path);
	st_mainloop_init();
	st_hook_signals();
	ipc = ipcsocket_init("/tmp/supla-server-ctrl.sock");

	// INI ACCEPT LOOP

	if ( ssd_ssl != NULL )
		ssl_accept_loop_t = sthread_simple_run(accept_loop, ssd_ssl, 0);

	if ( ssd_tcp != NULL )
		tcp_accept_loop_t = sthread_simple_run(accept_loop, ssd_tcp, 0);

	if ( ipc )
		ipc_accept_loop_t = sthread_simple_run(ipc_accept_loop, ipc, 0);

	// DATA LOGGER
	datalogger_loop_t = sthread_simple_run(datalogger_loop, NULL, 0);


	// MAIN LOOP

	while(st_app_terminate == 0) {
		st_mainloop_wait(1000000);
	}


	// RELEASE BLOCK

	if ( ipc != NULL ) {
		ipcsocket_close(ipc);
		sthread_twf(ipc_accept_loop_t);  // ! after ipcsocket_close and before ipcsocket_free !
		ipcsocket_free(ipc);
	}

	if ( ssd_ssl != NULL ) {
		ssocket_close(ssd_ssl);
		sthread_twf(ssl_accept_loop_t);  // ! after ssocket_close and before ssocket_free !
		ssocket_free(ssd_ssl);
	}

	if ( ssd_tcp != NULL ) {
		ssocket_close(ssd_tcp);
		sthread_twf(tcp_accept_loop_t);  // ! after ssocket_close and before ssocket_free !
		ssocket_free(ssd_tcp);
	}

	sthread_twf(datalogger_loop_t);

	st_mainloop_free();
	st_delpidfile(pidfile_path);

	supla_user::free();
	database::mainthread_end();

	scfg_free();

	return EXIT_SUCCESS;

exit_fail:

    ssocket_free(ssd_ssl);
    ssocket_free(ssd_tcp);
    scfg_free();
    exit(EXIT_FAILURE);

}
