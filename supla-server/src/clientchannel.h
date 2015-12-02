/*
 ============================================================================
 Name        : clientchannel.h
 Author      : Przemyslaw Zygmunt p.zygmunt@acsoftware.pl [AC SOFTWARE]
 Version     : 1.0
 Copyright   : GPLv2
 ============================================================================
 */

#ifndef CLIENTCHANNEL_H_
#define CLIENTCHANNEL_H_

#include "proto.h"

class supla_client;
class supla_client_channel {

	int Id;
	int DeviceId;
	int LocationId;
	int Func;
	int Param1;

	char *Caption;
	char remote_update;

public:
	supla_client_channel(int Id, int DeviceId, int LocationID, int Func, int Param1, const char *Caption);
	void setCaption(const char *Caption);
	void mark_for_remote_update(char mark);
	char marked_for_remote_update(void);
	void proto_get_channel(TSC_SuplaChannel *channel, supla_client *client);
	void proto_get_channel_value(TSC_SuplaChannelValue *channel_value, supla_client *client);
	virtual ~supla_client_channel();
	int getId();
	int getDeviceId();
};

class supla_client_channels {
private:
	supla_client *client;
	void *arr;

	static char arr_findcmp(void *ptr, void *id);
	static char arr_delcnd(void *ptr);
	void arr_clean(void);

	supla_client_channel *get_marked(void);
	supla_client_channel *find_channel(int Id);
	bool remote_update_cv(void *srpc);
	bool remote_update_c(void *srpc);

public:
	supla_client_channels(supla_client *client);
	virtual ~supla_client_channels();

	void update_channel(int Id, int DeviceId, int LocationID, int Func, int Param1, const char *Caption);
	void update_device_channels(int DeviceId);
	void on_channel_value_changed(void *srpc, int DeviceId, int ChannelId = 0);//ChannelId == 0 - All channels
    bool channel_exists(int ChannelID);
	void load(void);
	int count(void);
	bool remote_update(void *srpc);

	bool set_device_channel_new_value(TCS_SuplaChannelNewValue *channel_new_value);
};

#endif /* CLIENTCHANNEL_H_ */
