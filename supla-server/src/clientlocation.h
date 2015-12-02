/*
 ============================================================================
 Name        : clientlocation.h
 Author      : Przemyslaw Zygmunt p.zygmunt@acsoftware.pl [AC SOFTWARE]
 Version     : 1.0
 Copyright   : GPLv2
 ============================================================================
 */

#ifndef CLIENTLOCATION_H_
#define CLIENTLOCATION_H_

#include "proto.h"

class supla_client_location {
private:
	int Id;
	char *Caption;
public:
	supla_client_location(int Id, const char *Caption);
	virtual ~supla_client_location();
	int getId(void);
	void proto_get_location(TSC_SuplaLocation *location);
};

class supla_client_locations {
private:

	void *arr;
	// -------------

	void *lck;
	int *ids;
	int ids_count;

	void ids_clean();
	void arr_clean(void);
	static char arr_findcmp(void *ptr, void *id);
	static char arr_delcnd(void *ptr);

public:
	supla_client_locations();
	virtual ~supla_client_locations();
	void load(int ClientID);
	int count();
	void add_location(int Id, const char *Caption);
	bool remote_update(void *srpc);
	bool location_exists(int Id);
};

#endif /* CLIENTLOCATION_H_ */
