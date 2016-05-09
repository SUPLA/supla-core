/*
 ============================================================================
 Name        : user.cpp
 Author      : Przemyslaw Zygmunt p.zygmunt@acsoftware.pl [AC SOFTWARE]
 Version     : 1.0
 Copyright   : GPLv2
 ============================================================================
 */

#include <string.h>
#include <unistd.h>


#include "user.h"
#include "device.h"
#include "client.h"
#include "safearray.h"
#include "log.h"


void *supla_user::user_arr = NULL;

char supla_user::find_user_byid(void *ptr, void *UserID) {
	return ((supla_user*)ptr)->getUserID() == *(int*)UserID ? 1 : 0;
}

char supla_user::find_device_byid(void *ptr, void *ID) {
	return ((supla_device*)ptr)->getID() == *(int*)ID ? 1 : 0;
}

char supla_user::find_device_byguid(void *ptr, void *GUID) {
	return ((supla_device*)ptr)->cmpGUID((char*)GUID) ? 1 : 0;
}

char supla_user::find_client_byid(void *ptr, void *ID) {
	return ((supla_client*)ptr)->getID() == *(int*)ID ? 1 : 0;
}

char supla_user::find_client_byguid(void *ptr, void *GUID) {
	return ((supla_client*)ptr)->cmpGUID((char*)GUID) ? 1 : 0;
}


char supla_user_clean(void *ptr) {
	delete (supla_user*)ptr;
	return 1;
}

supla_user::supla_user(int UserID) {

	this->UserID = UserID;
	this->device_arr = safe_array_init();
	this->client_arr = safe_array_init();
	this->connections_allowed = true;

	safe_array_add(supla_user::user_arr, this);
}

supla_user::~supla_user() {
	safe_array_remove(supla_user::user_arr, this);


	safe_array_free(device_arr);
	safe_array_free(client_arr);
}

void supla_user::init(void) {
	supla_user::user_arr = safe_array_init();
}

void supla_user::free(void) {

	safe_array_clean(supla_user::user_arr, supla_user_clean);
	safe_array_free(supla_user::user_arr);
}

int supla_user::getUserID(void) {
	return UserID;
}

int supla_user::user_count(void) {
	return safe_array_count(supla_user::user_arr);
}

supla_user *supla_user::get_user(int idx) {
	return (supla_user *)safe_array_get(supla_user::user_arr, idx);
}

bool supla_user::getClientName(int ClientID, char *buffer, int size) {

	if ( size < 1 )
		return false;

	buffer[0] = 0;

	safe_array_lock(client_arr);

	supla_client *client = find_client(ClientID);

	if ( client )
       client->getName(buffer, size);

	safe_array_unlock(client_arr);

	return client != NULL;
}

supla_user *supla_user::find(int UserID, bool create) {

	safe_array_lock(supla_user::user_arr);

	supla_user *user =  (supla_user*)safe_array_findcnd(user_arr, find_user_byid, &UserID);

	if ( user == NULL )
       user = new supla_user(UserID);

	safe_array_unlock(supla_user::user_arr);

	return user;
}


supla_user *supla_user::add_device(supla_device *device, int UserID) {

	supla_user *user = find(UserID, true);

	safe_array_lock(user->device_arr);

	if ( safe_array_find(user->device_arr, device) == -1 ) {

		char GUID[SUPLA_GUID_SIZE];
		memset(GUID, 0, SUPLA_GUID_SIZE);
		device->getGUID(GUID);

		supla_device *_device = (supla_device *)safe_array_findcnd(user->device_arr, find_device_byguid, GUID);
		if ( _device != NULL ) {
			_device->terminate();
			usleep(1000000);
		}

		safe_array_add(user->device_arr, device);
	}

	safe_array_unlock(user->device_arr);

	return user;
}

supla_user *supla_user::add_client(supla_client *client, int UserID) {

	supla_user *user = find(UserID, true);

	safe_array_lock(user->client_arr);

	if ( safe_array_find(user->client_arr, client) == -1 ) {

		char GUID[SUPLA_GUID_SIZE];
		memset(GUID, 0, SUPLA_GUID_SIZE);
		client->getGUID(GUID);

		supla_client *_client = (supla_client *)safe_array_findcnd(user->client_arr, find_client_byguid, GUID);
		if ( _client != NULL ) {
			_client->terminate();
			usleep(1000000);
		}

		safe_array_add(user->client_arr, client);
	}

	safe_array_unlock(user->client_arr);

	return user;
}

void supla_user::remove_device(supla_device *device) {

	safe_array_remove(device_arr, device);

}

void supla_user::remove_client(supla_client *client) {

	safe_array_remove(client_arr, client);
}

supla_device *supla_user::find_device(int DeviceID) {
	return (supla_device *)safe_array_findcnd(device_arr, find_device_byid, &DeviceID);
}

supla_client *supla_user::find_client(int ClientID) {
	return (supla_client *)safe_array_findcnd(client_arr, find_client_byid, &ClientID);
}

bool supla_user::is_device_online(int DeviceID) {
	return find_device(DeviceID) != NULL;
}

bool supla_user::is_device_online(int UserID, int DeviceID) {

	bool result = false;

	safe_array_lock(supla_user::user_arr);

	supla_user *user =  (supla_user*)safe_array_findcnd(user_arr, find_user_byid, &UserID);

	if ( user
		  && user->is_device_online(DeviceID) == true )
		result = true;

	safe_array_unlock(supla_user::user_arr);

	return result;
}

bool supla_user::get_channel_double_value(int DeviceID, int ChannelID, double *Value, char Type) {

	bool result = false;

	safe_array_lock(device_arr);

	supla_device *device = find_device(DeviceID);
	if ( device != NULL ) {
		switch(Type) {
		case 0:
			result = device->get_channel_double_value(ChannelID, Value) == 1;
			break;
		case 1:
			result = device->get_channel_temperature_value(ChannelID, Value) == 1;
			break;
		case 2:
			result = device->get_channel_humidity_value(ChannelID, Value) == 1;
			break;
		}
	}

	safe_array_unlock(device_arr);

	return result;
}

bool supla_user::get_channel_double_value(int DeviceID, int ChannelID, double *Value) {
	return get_channel_double_value(DeviceID, ChannelID, Value, 0);
}

bool supla_user::get_channel_temperature_value(int DeviceID, int ChannelID, double *Value) {
	return get_channel_double_value(DeviceID, ChannelID, Value, 1);
}

bool supla_user::get_channel_humidity_value(int DeviceID, int ChannelID, double *Value) {
	return get_channel_double_value(DeviceID, ChannelID, Value, 2);
}

bool supla_user::get_channel_double_value(int UserID, int DeviceID, int ChannelID, double *Value, char Type) {

	bool result = false;

	safe_array_lock(supla_user::user_arr);

	supla_user *user =  (supla_user*)safe_array_findcnd(user_arr, find_user_byid, &UserID);

	if ( user ) {
		switch(Type) {
		case 0:
			result = user->get_channel_double_value(DeviceID, ChannelID, Value) == true;
			break;
		case 1:
			result = user->get_channel_temperature_value(DeviceID, ChannelID, Value) == true;
			break;
		case 2:
			result = user->get_channel_humidity_value(DeviceID, ChannelID, Value) == true;
			break;
		}
	}

	safe_array_unlock(supla_user::user_arr);

	return result;

}

bool supla_user::get_channel_double_value(int UserID, int DeviceID, int ChannelID, double *Value) {
     return get_channel_double_value(UserID, DeviceID, ChannelID, Value, 0);
}

bool supla_user::get_channel_temperature_value(int UserID, int DeviceID, int ChannelID, double *Value) {
    return get_channel_double_value(UserID, DeviceID, ChannelID, Value, 1);
}

bool supla_user::get_channel_humidity_value(int UserID, int DeviceID, int ChannelID, double *Value) {
    return get_channel_double_value(UserID, DeviceID, ChannelID, Value, 2);
}

supla_device *supla_user::device_by_channel_id(supla_device *suspect, int ChannelID) {

	if ( suspect != NULL
			&& suspect->channel_exists(ChannelID) ) return suspect;

	for(int a=0;a<safe_array_count(device_arr);a++)
		if ( NULL != ( suspect = (supla_device *)safe_array_get(device_arr, a) )
			 && suspect->channel_exists(ChannelID) ) return suspect;

	return NULL;
}


bool supla_user::get_channel_value(int DeviceID, int ChannelID, TSuplaChannelValue *value, char *online) {

	bool result = false;
	memset(value, 0, sizeof(TSuplaChannelValue));
	*online = 0;

	safe_array_lock(device_arr);

	supla_device *device = find_device(DeviceID);
	if ( device ) {

		*online = 1;
		result = device->get_channel_value(ChannelID, value->value);

		if ( result ) {
			int slave_channel_id = device->slave_channel(ChannelID);

			if ( slave_channel_id != 0 ) {

				device = device_by_channel_id(device, slave_channel_id);

				if ( device ) {
					device->get_channel_value(slave_channel_id, value->sub_value);
				}

			}
		}
	}


	safe_array_unlock(device_arr);

	return result;
}

bool supla_user::set_device_channel_value(int SenderID, int DeviceID, int ChannelID, const char value[SUPLA_CHANNELVALUE_SIZE]) {

	bool result = false;

	safe_array_lock(device_arr);

	supla_device *device = find_device(DeviceID);
	if ( device )
		device->set_device_channel_value(SenderID, ChannelID, value);

	safe_array_unlock(device_arr);

	return result;
}

void supla_user::update_client_device_channels(int LocationID, int DeviceID) {

	safe_array_lock(client_arr);
	{
		supla_client *client;

		for(int a=0;a<safe_array_count(client_arr);a++)
			if ( NULL != ( client = (supla_client *)safe_array_get(client_arr, a) ) ) {
				client->update_device_channels(LocationID, DeviceID);
			}

	}
	safe_array_unlock(client_arr);
}

void supla_user::on_channel_value_changed(int DeviceId, int ChannelId) {

	safe_array_lock(device_arr);
	{
		supla_device *device = find_device(DeviceId);
		if ( device ) {

				int master_channel_id = device->master_channel(ChannelId);

				if ( master_channel_id != 0 ) {
					DeviceId = device->getID();
					ChannelId = master_channel_id;
				}
		}
	}
	safe_array_unlock(device_arr);

	safe_array_lock(client_arr);
	{
		supla_client *client;

		for(int a=0;a<safe_array_count(client_arr);a++)
			if ( NULL != ( client = (supla_client *)safe_array_get(client_arr, a) ) ) {
				client->on_channel_value_changed(DeviceId, ChannelId);
			}

	}
	safe_array_unlock(client_arr);

}

void supla_user::call_event(TSC_SuplaEvent *event) {

	safe_array_lock(client_arr);
	{
		supla_client *client;

		for(int a=0;a<safe_array_count(client_arr);a++)
			if ( NULL != ( client = (supla_client *)safe_array_get(client_arr, a) ) ) {
				client->call_event(event);
			}

	}
	safe_array_unlock(client_arr);

}

void supla_user::get_temp_and_humidity(void *tarr) {

	int a;

	safe_array_lock(device_arr);

	for(a=0;a<safe_array_count(device_arr);a++) {
		((supla_device *)safe_array_get(device_arr, a))->get_temp_and_humidity(tarr);
	}

	safe_array_unlock(device_arr);

}


void supla_user::reconnect() {

	int a;

	safe_array_lock(device_arr);
	{
		supla_device *device;

		for(a=0;a<safe_array_count(device_arr);a++)
			if ( NULL != ( device = (supla_device *)safe_array_get(device_arr, a) ) ) {
				device->terminate();
			}

	}
	safe_array_unlock(device_arr);

	safe_array_lock(client_arr);
	{
		supla_client *client;

		for(a=0;a<safe_array_count(client_arr);a++)
			if ( NULL != ( client = (supla_client *)safe_array_get(client_arr, a) ) ) {
				client->terminate();
			}

	}
	safe_array_unlock(client_arr);

}

bool supla_user::reconnect(int UserID) {

	supla_user *user = find(UserID, true);

	if ( user ) {
		user->reconnect();
		return true;
	}

	return false;

}


