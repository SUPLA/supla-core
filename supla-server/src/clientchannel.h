/*
 Copyright (C) AC SOFTWARE SP. Z O.O.

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
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
	int AltIcon;
	unsigned char ProtocolVersion;
	unsigned int Flags;

	char *Caption;
	char remote_update;

public:
	supla_client_channel(int Id, int DeviceId, int LocationID, int Func, int Param1, const char *Caption, int AltIcon, unsigned char ProtocolVersion, bool Hidden);
	void setCaption(const char *Caption);
	void mark_for_remote_update(char mark);
	char marked_for_remote_update(void);
	void proto_get_channel(TSC_SuplaChannel *channel, supla_client *client);
	void proto_get_channel(TSC_SuplaChannel_B *channel, supla_client *client);
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
	bool remote_update_c_b(void *srpc);
public:
	supla_client_channels(supla_client *client);
	virtual ~supla_client_channels();

	void update_channel(int Id, int DeviceId, int LocationID, int Func, int Param1, const char *Caption, int AltIcon, unsigned char ProtocolVersion, bool Hidden);
	void update_device_channels(int DeviceId);
	void on_channel_value_changed(void *srpc, int DeviceId, int ChannelId = 0);//ChannelId == 0 - All channels
    bool channel_exists(int ChannelID);
	void load(void);
	int count(void);
	bool remote_update(void *srpc);

	bool set_device_channel_new_value(TCS_SuplaChannelNewValue_B *channel_new_value);
};

#endif /* CLIENTCHANNEL_H_ */
