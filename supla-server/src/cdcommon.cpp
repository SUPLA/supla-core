/*
 ============================================================================
 Name        : cdcommon.cpp
 Author      : Przemyslaw Zygmunt p.zygmunt@acsoftware.pl [AC SOFTWARE]
 Version     : 1.0
 Copyright   : GPLv2
 ============================================================================
 */

#include <string.h>

#include "cdcommon.h"
#include "lck.h"

cdcommon::cdcommon(serverconnection *svrconn) {

	this->user = NULL;
	this->svrconn = svrconn;
	this->lck = lck_init();
	this->ID = 0;
	memset(this->GUID, 0, SUPLA_GUID_SIZE);

	updateLastActivity(); // last line / after lck_init
}

cdcommon::~cdcommon() {
	lck_free(this->lck);
}

void cdcommon::terminate(void) {
	lck_lock(lck);
	if ( svrconn )
		svrconn->terminate();
	lck_unlock(lck);
}

bool cdcommon::setGUID(char GUID[SUPLA_GUID_SIZE]) {

	char _GUID[SUPLA_GUID_SIZE];
	memset(_GUID, 0 , SUPLA_GUID_SIZE);

	if ( memcmp(_GUID, GUID, SUPLA_GUID_SIZE) == 0 )
		return false;

	lck_lock(lck);
	memcpy(this->GUID, GUID, SUPLA_GUID_SIZE);
	lck_unlock(lck);

	return true;
}

void cdcommon::getGUID(char GUID[SUPLA_GUID_SIZE]) {

	lck_lock(lck);
	memcpy(GUID, this->GUID, SUPLA_GUID_SIZE);
	lck_unlock(lck);
}

bool cdcommon::cmpGUID(const char GUID[SUPLA_GUID_SIZE]) {

	bool result = false;

	lck_lock(lck);
	result = memcmp(this->GUID, GUID, SUPLA_GUID_SIZE) == 0;
	lck_unlock(lck);

	return result;
}

int cdcommon::getID(void) {

	int result = false;

	lck_lock(lck);
	result = ID;
	lck_unlock(lck);

	return result;

}

void cdcommon::setID(int ID) {

	lck_lock(lck);
	this->ID = ID;
	lck_unlock(lck);

}

void cdcommon::setUser(supla_user *user) {
	lck_lock(lck);
	this->user = user;
	lck_unlock(lck);
}

supla_user *cdcommon::getUser(void) {

	supla_user *result;

	lck_lock(lck);
	result = this->user;
	lck_unlock(lck);

	return result;
}


serverconnection * cdcommon::getSvrConn(void) {
	return svrconn;
}

void cdcommon::updateLastActivity(void) {
	lck_lock(lck);
	gettimeofday(&last_activity_time, NULL);
	lck_unlock(lck);
}

int cdcommon::getActivityDelay(void) {

	int result;
	struct timeval now;
	gettimeofday(&now, NULL);

	lck_lock(lck);
	result = now.tv_sec-last_activity_time.tv_sec;
	lck_unlock(lck);

	return result;
}
