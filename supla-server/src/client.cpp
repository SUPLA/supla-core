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

#include <stdlib.h>
#include <string.h>

#include "client.h"
#include "safearray.h"
#include "user.h"
#include "database.h"
#include "srpc.h"
#include "lck.h"
#include "log.h"
#include "clientlocation.h"

supla_client::supla_client(serverconnection *svrconn)
             :cdcommon(svrconn)  {

    this->locations = new supla_client_locations();
    this->channels = new supla_client_channels(this);
    this->name[0] = 0;
}

supla_client::~supla_client() {

	if ( getUser() ) // 1st line !!!
		getUser()->remove_client(this);

	delete channels;
	delete locations;

}

void supla_client::setName(const char *name) {

	lck_lock(lck);
	snprintf(this->name, SUPLA_CLIENT_NAME_MAXSIZE, "%s", name);
	this->name[SUPLA_CLIENT_NAME_MAXSIZE-1] = 0;
	lck_unlock(lck);

}

int supla_client::getName(char *buffer, int size) {

	if ( size < 1 )
		return 0;

	buffer[0] = 0;

	lck_lock(lck);
	snprintf(buffer, size, "%s", this->name);
	lck_unlock(lck);

	buffer[size-1] = 0;
	return strlen(buffer);
}

char supla_client::register_client(TCS_SuplaRegisterClient *register_client, unsigned char proto_version) {

	int resultcode = SUPLA_RESULTCODE_TEMPORARILY_UNAVAILABLE;
	char result = 0;

	if ( !setGUID(register_client->GUID) ) {
		resultcode = SUPLA_RESULTCODE_GUID_ERROR;
	} else {

		database *db = new database();

		if ( db->connect() == true ) {

			int UserID;
			int Limit;
			bool is_enabled = true;

			if ( db->accessid_auth(register_client->AccessID, register_client->AccessIDpwd, &UserID, &is_enabled, &Limit) == false ) {
				resultcode = is_enabled ? SUPLA_RESULTCODE_BAD_CREDENTIALS : SUPLA_RESULTCODE_ACCESSID_DISABLED;
			} else {

				is_enabled = true;

				db->start_transaction();

				int ClientID = db->add_client(register_client->AccessID, register_client->GUID, register_client->Name,
						getSvrConn()->getClientIpv4(), register_client->SoftVer, proto_version, UserID, &is_enabled, &Limit);

				if ( ClientID == 0 ) {

					db->rollback();

					if ( Limit <= 0 ) {
						resultcode = SUPLA_RESULTCODE_CLIENT_LIMITEXCEEDED;
					} else if ( !is_enabled ) {
						resultcode = SUPLA_RESULTCODE_CLIENT_DISABLED;
					}

				} else {

					db->commit();

					if ( is_enabled ) {

						setID(ClientID);
						setName(register_client->Name);

						loadConfig();

						resultcode = SUPLA_RESULTCODE_TRUE;
						result = 1;
						setUser(supla_user::add_client(this, UserID));

					} else {
						resultcode = SUPLA_RESULTCODE_CLIENT_DISABLED;
					}
				}



			}

		}

		delete db;

	}

	TSC_SuplaRegisterClientResult srcr;
	srcr.result_code = resultcode;
	srcr.ClientID = getID();
	srcr.activity_timeout = getSvrConn()->GetActivityTimeout();
	srcr.version_min = SUPLA_PROTO_VERSION;
	srcr.version = SUPLA_PROTO_VERSION;
	srcr.LocationCount = locations->count();
	srcr.ChannelCount = channels->count();
	srpc_sc_async_registerclient_result(getSvrConn()->srpc(), &srcr);

	// !After srpc_async_registerclient_result
	if ( resultcode == SUPLA_RESULTCODE_TRUE ) {
		remote_update_lists();
	}


	return result;
}

void supla_client::update_device_channels(int LocationID, int DeviceID) {

	if ( locations->location_exists(LocationID) ) {
		channels->update_device_channels(DeviceID);
		remote_update_lists();
	}

}

void supla_client::on_channel_value_changed(int DeviceId, int ChannelId) {

	channels->on_channel_value_changed(getSvrConn()->srpc(), DeviceId, ChannelId);

}


void supla_client::remote_update_lists(void) {

	if ( locations->remote_update(getSvrConn()->srpc()) )
		return;

	if ( channels->remote_update(getSvrConn()->srpc()) )
		return;
}


void supla_client::loadConfig(void) {

	locations->load(getID());
	channels->load();
}

void supla_client::get_next(void) {
	remote_update_lists();
}

void supla_client::set_device_channel_new_value(TCS_SuplaChannelNewValue_B *channel_new_value) {

	channels->set_device_channel_new_value(channel_new_value);
}

void supla_client::call_event(TSC_SuplaEvent *event) {
	srpc_sc_async_event(getSvrConn()->srpc(), event);
}
