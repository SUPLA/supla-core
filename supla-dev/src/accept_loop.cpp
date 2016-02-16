/*
 ============================================================================
 Name        : accept_loop.cpp
 Author      : Przemyslaw Zygmunt p.zygmunt@acsoftware.pl [AC SOFTWARE]
 Version     : 1.0
 Copyright   : GPLv2
 ============================================================================
 */

#include <unistd.h>
#include <stdio.h>

#include "supla-socket.h"
#include "ipcsocket.h"
#include "sthread.h"
#include "safearray.h"
#include "log.h"
#include "ipcctrl.h"

void accept_loop_ipcctrl_finish(void *ipcctrl, void *sthread) {

	delete (svr_ipcctrl *)ipcctrl;

}

void accept_loop_ipcctrl_execute(void *ipcctrl, void *sthread) {

	((svr_ipcctrl *)ipcctrl)->execute(sthread);

}

char accept_loop_ipcctrl_thread_cnd(void *ipcctrl_sthread) {

   if ( sthread_isfinished(ipcctrl_sthread) == 1 ) {

	   sthread_free(ipcctrl_sthread);
	   return 1;
   }

   return 0;
}

char accept_loop_ipcctrl_thread_twt(void *ipcctrl_sthread) {
	sthread_twf(ipcctrl_sthread);
	return 1;
}

void ipc_accept_loop(void *ipc, void *ipc_al_sthread) {

	int client_sd;
	void *ipcctrl_thread_arr = safe_array_init();

	while(sthread_isterminated(ipc_al_sthread) == 0) {

		safe_array_clean(ipcctrl_thread_arr, accept_loop_ipcctrl_thread_cnd);

		if ( -1 == (client_sd = ipcsocket_accept(ipc)) ) {
			break;
		} else {

			Tsthread_params stp;

			stp.execute = accept_loop_ipcctrl_execute;
			stp.finish = accept_loop_ipcctrl_finish;
			stp.user_data = new svr_ipcctrl(client_sd);
			stp.free_on_finish = 0;
			stp.initialize = NULL;

			safe_array_add(ipcctrl_thread_arr, sthread_run(&stp));

		}
	}

	safe_array_clean(ipcctrl_thread_arr, accept_loop_ipcctrl_thread_twt);
	safe_array_free(ipcctrl_thread_arr);

}
