/*
 ============================================================================
 Name        : accept_loop.h
 Author      : Przemyslaw Zygmunt p.zygmunt@acsoftware.pl [AC SOFTWARE]
 Version     : 1.0
 Copyright   : GPLv2
 ============================================================================
 */

#ifndef ACCEPT_LOOP_H_
#define ACCEPT_LOOP_H_

void accept_loop(void *ssd, void *sthread);
void ipc_accept_loop(void *ipc, void *ipc_al_sthread);



#endif /* ACCEPT_LOOP_H_ */
