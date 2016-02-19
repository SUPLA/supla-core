/*
 ============================================================================
 Name        : user.h
 Author      : Przemyslaw Zygmunt p.zygmunt@acsoftware.pl [AC SOFTWARE]
 Version     : 1.0
 Copyright   : GPLv2
 ============================================================================
 */

#ifndef USER_H_
#define USER_H_

#include "proto.h"

class supla_device;
class supla_client;

class supla_user {
protected:

	static void *user_arr;

	void *device_arr;
	void *client_arr;
	int UserID;
	bool connections_allowed;

	supla_device *find_device(int DeviceID);
	supla_client *find_client(int ClientID);
	supla_device *device_by_channel_id(supla_device *suspect, int ChannelID);

	static char find_user_byid(void *ptr, void *UserID);
    static char find_device_byid(void *ptr, void *ID);
    static char find_device_byguid(void *ptr, void *GUID);
    static char find_client_byid(void *ptr, void *ID);
    static char find_client_byguid(void *ptr, void *GUID);

    void reconnect();

public:


    static void init(void);
    static void free(void);
    static supla_user *add_device(supla_device *device, int UserID);
    static supla_user *add_client(supla_client *client, int UserID);
    static supla_user *find(int UserID, bool create);
    static bool reconnect(int UserID);
    static bool is_device_online(int UserID, int DeviceID);
    static bool get_channel_double_value(int UserID, int DeviceID, int ChannelID, double *Value);
    static int user_count(void);
    static supla_user *get_user(int idx);

    void remove_device(supla_device *device);
    void remove_client(supla_client *client);

    int getUserID(void);
    bool getClientName(int ClientID, char *buffer, int size);

    bool get_channel_double_value(int DeviceID, int ChannelID, double *Value);
    bool is_device_online(int DeviceID);
    bool get_channel_value(int DeviceID, int ChannelID, TSuplaChannelValue *value, char *online);
    bool set_device_channel_value(int SenderID, int DeviceID, int ChannelID, const char value[SUPLA_CHANNELVALUE_SIZE]);
    void update_client_device_channels(int LocationID, int DeviceID);
    void on_channel_value_changed(int DeviceId, int ChannelId = 0);

    void call_event(TSC_SuplaEvent *event);
    void get_temperatures(void *tarr);

	supla_user(int UserID);
	virtual ~supla_user();
};

#endif /* USER_H_ */
