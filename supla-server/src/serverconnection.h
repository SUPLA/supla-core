/*
 ============================================================================
 Name        : serverconnection.h
 Author      : Przemyslaw Zygmunt p.zygmunt@acsoftware.pl [AC SOFTWARE]
 Version     : 1.0
 Copyright   : GPLv2
 ============================================================================
 */

#ifndef SERVERCONNECTION_H_
#define SERVERCONNECTION_H_

#include <sys/time.h>
#include <stddef.h>
#include "eh.h"


class supla_client;
class supla_device;
class cdcommon;

class serverconnection {
protected:

	unsigned int client_ipv4;
	void *ssd;
	void *supla_socket;
	void *_srpc;
	void *sthread;
	TEventHandler *eh;

	struct timeval init_time;
	unsigned char activity_timeout;


	union {
	supla_client *client;
	supla_device *device;
	cdcommon *cdptr;
	};

	char registered;

public:
	serverconnection(void *ssd, void *supla_socket, unsigned int client_ipv4);
	void execute(void *sthread);
	void terminate(void);
	virtual ~serverconnection();

	int socket_read(void *buf, size_t count);
	int socket_write(const void *buf, size_t count);
	void on_remote_call_received(void *_srpc, unsigned int rr_id, unsigned int call_type, unsigned char proto_version);
	void *srpc(void);
	unsigned int getClientIpv4(void);
	unsigned char GetActivityTimeout(void);

};


#endif /* SERVERCONNECTION_H_ */
