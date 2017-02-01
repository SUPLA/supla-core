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

#define IPC_AUTH_TYPE_OAUTH_TOKEN          1

#define IPC_AUTH_LEVEL_UNAUTHORIZED        0

#define IPC_AUTH_LEVEL_OAUTH_USER          1
#define IPC_AUTH_LEVEL_SUPERUSER           2

#include "eh.h"


class svr_ipcctrl {
private:
	int sfd;
	TEventHandler *eh;

	char auth_level;
	int auth_expires_at;
	int user_id;
	int oauth_user_id;

	void set_unauthorized(void);
	void oauth(const char *cmd);
	bool is_authorized(char level, int UserID, bool _send_result);

	bool match_command(const char *cmd, int len);
	void get_double(const char *cmd, char Type);
	void get_char(const char *cmd);
	void get_rgbw(const char *cmd);
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
