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

#ifndef DB_H_
#define DB_H_

#include "proto.h"
#include "device.h"
#include "client.h"

class database {
private:
	void *_mysql;
	int query(const char *stmt_str, bool log_err = false);
	bool stmt_execute(void **_stmt, const char *stmt_str, void *bind, int bind_size, bool exec_errors = false);
	bool stmt_get_int(void **_stmt, int *value1, int *value2, int *value3, int *value4, const char *stmt_str, void *bind, int bind_size, bool exec_errors = false);
	void stmt_close(void *_stmt);
	bool auth(const char *query, int ID, char *_PWD, int _PWD_HEXSIZE, int *UserID, bool *is_enabled, int *limit);
	int get_count(int ID, const char *sql);

public:
	database();
	bool connect(void);
	void disconnect(void);
	virtual ~database();

	static bool mainthread_init(void);
	static void mainthread_end(void);

	static void thread_init(void);
	static void thread_end(void);

	void start_transaction(void);
	void commit(void);
	void rollback(void);


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

#endif /* DB_H_ */
