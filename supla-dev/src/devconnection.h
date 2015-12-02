/*
 ============================================================================
 Name        : dev-connection.h
 Author      : Przemyslaw Zygmunt p.zygmunt@acsoftware.pl [AC SOFTWARE]
 Version     : 1.0
 Copyright   : GPLv2
 ============================================================================
 */

#ifndef DEV_CONNECTION_H_
#define DEV_CONNECTION_H_

void* devconnection_start(void);
void devconnection_stop(void *dconn_thread);

#endif /* DEV_CONNECTION_H_ */
