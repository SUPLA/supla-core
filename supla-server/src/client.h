/*
 ============================================================================
 Name        : client.h
 Author      : Przemyslaw Zygmunt p.zygmunt@acsoftware.pl [AC SOFTWARE]
 Version     : 1.0
 Copyright   : GPLv2
 ============================================================================
 */

#ifndef CLIENT_H_
#define CLIENT_H_

#include "cdcommon.h"
#include "clientlocation.h"
#include "clientchannel.h"

class supla_user;
class supla_client : public cdcommon {
private:
	char name[SUPLA_CLIENT_NAME_MAXSIZE];
protected:

	supla_client_locations *locations;
	supla_client_channels *channels;

	void loadIODevices(void);
    void loadConfig(void);

    void remote_update_lists(void);
    void setName(const char *name);

public:
	supla_client(serverconnection *svrconn);

	char register_client(TCS_SuplaRegisterClient *register_client, unsigned char proto_version);
	void update_device_channels(int LocationID, int DeviceID);
	void on_channel_value_changed(int DeviceId, int ChannelId = 0);

	void get_next(void);
	void set_device_channel_new_value(TCS_SuplaChannelNewValue_B *channel_new_value);
	int getName(char *buffer, int size) ;

	void call_event(TSC_SuplaEvent *event);

	virtual ~supla_client();
};

#endif /* CLIENT_H_ */
