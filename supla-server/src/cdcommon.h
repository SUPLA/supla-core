/*
 ============================================================================
 Name        : cdcommon.h
 Author      : Przemyslaw Zygmunt p.zygmunt@acsoftware.pl [AC SOFTWARE]
 Version     : 1.0
 Copyright   : GPLv2
 ============================================================================
 */

#ifndef CDCOMMON_H_
#define CDCOMMON_H_

#include "proto.h"
#include "serverconnection.h"

class supla_user;
class cdcommon {

private:
	struct timeval last_activity_time;
	char GUID[SUPLA_GUID_SIZE];
	serverconnection *svrconn;
	int ID;
	supla_user *user;

protected:

    void *lck;


    // Thread safe start
    bool setGUID(char GUID[SUPLA_GUID_SIZE]);
    void setID(int ID);
    void setUser(supla_user *user);
    // Thread safe end

    serverconnection *getSvrConn(void);

public:
	cdcommon(serverconnection *svrconn);
	virtual ~cdcommon();


    // Thread safe start
	void terminate(void);
    void getGUID(char GUID[SUPLA_GUID_SIZE]);
    int getID(void);
    supla_user *getUser(void);
    bool cmpGUID(const char GUID1[SUPLA_GUID_SIZE]);
    void updateLastActivity(void);
    int getActivityDelay(void);
    // Thread safe end
};

#endif /* CDCOMMON_H_ */
