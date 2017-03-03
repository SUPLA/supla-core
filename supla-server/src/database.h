/*
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

#ifndef DATABASE_H_
#define DATABASE_H_

#include "db.h"
#include "proto.h"
#include "device.h"
#include "client.h"

class database : public dbcommon {
private:

	virtual char *cfg_get_host(void);
	virtual char *cfg_get_user(void);
	virtual char *cfg_get_password(void);
	virtual char *cfg_get_database(void);
	virtual int cfg_get_port(void);

	bool auth(const char *query, int ID, char *_PWD, int _PWD_HEXSIZE, int *UserID, bool *is_enabled, int *limit);
public:

	bool location_auth(int LocationID, char *LocationPWD, int *UserID, bool *is_enabled, int *limit_iodev);
	bool accessid_auth(int AccessID, char *AccessIDpwd, int *UserID, bool *is_enabled, int *limit_client);

	int add_device(int *LocationID, const char GUID[SUPLA_GUID_SIZE], const char Name[SUPLA_DEVICE_NAME_MAXSIZE],
			        unsigned int ipv4, char softver[SUPLA_SOFTVER_MAXSIZE], int proto_version,
			        int UserID, bool *new_device, bool *is_enabled, int *Limit);

	int add_channel(int DeviceID, int ChannelNumber, int ChannelType);
	int add_device_channel(int DeviceID, int ChannelNumber, int Type, int Func, int FList, int UserID, bool *new_channel);

	int get_device_count(int UserID);
	int get_device_id(const char GUID[SUPLA_GUID_SIZE], int *location_id, bool *is_enabled);
	int get_device_channel_id(int DeviceID, int ChannelNumber, int *Type);
	int get_device_channel_count(int DeviceID);
	int get_device_channel_type(int DeviceID, int ChannelNumber);
	void get_device_channels(int DeviceID, supla_device_channels *channels);

	bool get_device_firmware_update_url(int DeviceID, TDS_FirmwareUpdateParams *params, TSD_FirmwareUpdate_UrlResult *url);

	bool on_newdevice(int DeviceID);
	bool on_channeladded(int DeviceID, int ChannelID);

	int get_client_count(int UserID);
	int get_client_id(const char GUID[SUPLA_GUID_SIZE], int access_id, bool *is_enabled);
	int add_client(int AccessID, const char GUID[SUPLA_GUID_SIZE], const char Name[SUPLA_DEVICE_NAME_MAXSIZE],
			                   unsigned int ipv4, char softver[SUPLA_SOFTVER_MAXSIZE], int proto_version, int UserID, bool *is_enabled, int *Limit);

	void get_client_locations(int ClientID, supla_client_locations *locs);
	void get_client_channels(int ClientID, int *DeviceID, supla_client_channels *channels);

	void add_temperature(int ChannelID, double temperature);
	void add_temperature_and_humidity(int ChannelID, double temperature, double humidity);


	bool get_oauth_user(char *access_token, int *OAuthUserID, int *UserID, int *expires_at);

};

#endif /* DATABASE_H_ */
