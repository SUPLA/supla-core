/*
 ============================================================================
 Name        : ipcctrl.h
 Author      : Przemyslaw Zygmunt p.zygmunt@acsoftware.pl [AC SOFTWARE]
 Version     : 1.0
 Copyright   : GPLv2
 ============================================================================
 */


#ifndef IPCCTRL_H_
#define IPCCTRL_H_

#include "eh.h"

class svr_ipcctrl {
private:
	int sfd;
	TEventHandler *eh;
	bool match_command(const char *cmd, int len);
	void cmd_get_double(const char *cmd, char Type) ;
	void send_result(const char *result);
	void send_result(const char *result, int i);
	void send_result(const char *result, double i);
	char buffer[256];
public:
	svr_ipcctrl(int sfd);
	void execute(void *sthread);
	virtual ~svr_ipcctrl();
};

#endif /* IPCCTRL_H_ */
