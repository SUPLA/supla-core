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

#include <string.h>
#include <stdlib.h>

#include "device.h"
#include "safearray.h"
#include "unistd.h"
#include "db.h"
#include "srpc.h"
#include "log.h"
#include "user.h"
#include "lck.h"


supla_device::supla_device(serverconnection *svrconn)
	   :cdcommon(svrconn)  {

    this->channels = new supla_device_channels();
}

supla_device::~supla_device() {

	if ( getUser() ) { // 1st line!
		getUser()->remove_device(this);
		getUser()->on_channel_value_changed(getID());
	}

	delete channels;

}

char supla_device::register_device(TDS_SuplaRegisterDevice_B *register_device, unsigned char proto_version) {

	int resultcode = SUPLA_RESULTCODE_TEMPORARILY_UNAVAILABLE;
	char result = 0;

	if ( !setGUID(register_device->GUID) ) {
		resultcode = SUPLA_RESULTCODE_GUID_ERROR;
	} else {

		database *db = new database();

		if ( db->connect() == true ) {

			int UserID;
			int Limit;
			bool LocationEnabled = true;

			if ( db->location_auth(register_device->LocationID, register_device->LocationPWD, &UserID, &LocationEnabled, &Limit) == false ) {
				resultcode = LocationEnabled ? SUPLA_RESULTCODE_BAD_CREDENTIALS : SUPLA_RESULTCODE_LOCATION_DISABLED;
			} else {

				db->start_transaction();
				bool new_device = false;
				bool is_enabled = false;

				if ( strlen(register_device->Name) < 1 ) {
					snprintf(register_device->Name, SUPLA_DEVICE_NAME_MAXSIZE, "unknown");
				}

				int LocationID = register_device->LocationID;
				int DeviceID = db->add_device(&LocationID,
						                      register_device->GUID,
						                      register_device->Name,
						                      getSvrConn()->getClientIpv4(),
						                      register_device->SoftVer,
						                      proto_version,
						                      UserID,
						                      &new_device,
						                      &is_enabled,
						                      &Limit);

				if ( DeviceID == 0 ) {

					db->rollback();

					if ( Limit <= 0 ) {
						resultcode = SUPLA_RESULTCODE_DEVICE_LIMITEXCEEDED;
					} else if ( is_enabled == false ) {
						resultcode = SUPLA_RESULTCODE_DEVICE_DISABLED;
					} else if ( LocationID == 0 ) {
						resultcode = SUPLA_RESULTCODE_LOCATION_CONFLICT;
				    }

				} else {

					int ChannelCount = 0;
					int ChannelType;

					for(int a=0;a<SUPLA_CHANNELMAXCOUNT;a++)
						if ( a >= register_device->channel_count ) {
							break;
						} else {
							ChannelCount++;
							if ( db->get_device_channel_id(DeviceID, register_device->channels[a].Number, &ChannelType) == 0 ) {
								ChannelType = 0;
							}

							if ( ChannelType == 0 ) {

								bool new_channel = false;
								int ChannelID = db->add_device_channel(DeviceID, register_device->channels[a].Number,
										                                         register_device->channels[a].Type,
										                                         register_device->channels[a].Default ? register_device->channels[a].Default : 0,
										                                         register_device->channels[a].FuncList,
										                                         UserID, &new_channel);

								if ( ChannelID == 0 ) {
									ChannelCount = -1;
									break;
								} else if ( new_channel ) {
									db->on_channeladded(DeviceID, ChannelID);
								}

							} else if ( ChannelType != register_device->channels[a].Type ) {
								ChannelCount = -1;
								break;
							}
						}

					if ( ChannelCount == -1
						 || db->get_device_channel_count(DeviceID) != ChannelCount ) {

						db->rollback();
						resultcode = SUPLA_RESULTCODE_CHANNEL_CONFLICT;

					} else {

						if ( new_device )
							db->on_newdevice(DeviceID);

						db->commit();

						setID(DeviceID);

						load_config();

						channels->set_channels_value(register_device->channels, register_device->channel_count);

						resultcode = SUPLA_RESULTCODE_TRUE;
						result = 1;
						setUser(supla_user::add_device(this, UserID));
						getUser()->update_client_device_channels(LocationID, getID());

					}
				}
			}

		}

		delete db;

	}

	if ( resultcode != SUPLA_RESULTCODE_TRUE ) {
		usleep(2000000);
	}

	TSD_SuplaRegisterDeviceResult srdr;
	srdr.result_code = resultcode;
	srdr.activity_timeout = getSvrConn()->GetActivityTimeout();
	srdr.version_min = SUPLA_PROTO_VERSION;
	srdr.version = SUPLA_PROTO_VERSION;
	srpc_sd_async_registerdevice_result(getSvrConn()->srpc(), &srdr);

	return result;
}

void supla_device::load_config(void) {
	channels->load(getID());
}

void supla_device::on_device_channel_value_changed(TDS_SuplaDeviceChannelValue *value) {

	int ChannelId = channels->get_channel_id(value->ChannelNumber);

	if ( ChannelId != 0 ) {
		channels->set_channel_value(ChannelId, value->value);
		getUser()->on_channel_value_changed(getID(), ChannelId);
	}
}

void supla_device::on_channel_set_value_result(TDS_SuplaChannelNewValueResult *result) {

	int ChannelID;

	if ( result->Success == 1
		 && ( ChannelID = channels->get_channel_id(result->ChannelNumber) ) != 0 ) {

		TSC_SuplaEvent event;
		memset(&event, 0, sizeof(TSC_SuplaEvent));
		event.ChannelID = ChannelID;
		event.SenderID = result->SenderID;
		event.DurationMS = channels->get_channel_value_duration(ChannelID);
		getUser()->getClientName(result->SenderID, event.SenderName, SUPLA_SENDER_NAME_MAXSIZE);
		event.SenderNameSize = strlen(event.SenderName)+1;

		switch(channels->get_channel_func(ChannelID)) {

		case SUPLA_CHANNELFNC_CONTROLLINGTHEGATEWAYLOCK:
			event.Event = SUPLA_EVENT_CONTROLLINGTHEGATEWAYLOCK;
			break;
		case SUPLA_CHANNELFNC_CONTROLLINGTHEGATE:
			event.Event = SUPLA_EVENT_CONTROLLINGTHEGATE;
			break;
		case SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR:
			event.Event = SUPLA_EVENT_CONTROLLINGTHEGARAGEDOOR;
			break;
		case SUPLA_CHANNELFNC_CONTROLLINGTHEDOORLOCK:
			event.Event = SUPLA_EVENT_CONTROLLINGTHEDOORLOCK;
			break;
		case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER:
			event.Event = SUPLA_EVENT_CONTROLLINGTHEROLLERSHUTTER;
			break;
		case SUPLA_CHANNELFNC_POWERSWITCH:
			event.Event = SUPLA_EVENT_POWERONOFF;
		    break;
		case SUPLA_CHANNELFNC_LIGHTSWITCH:
		     event.Event = SUPLA_EVENT_LIGHTONOFF;
		    break;

		}

		getUser()->call_event(&event);
	}

}

bool supla_device::get_channel_value(int ChannelID, char value[SUPLA_CHANNELVALUE_SIZE]) {
	return channels->get_channel_value(ChannelID, value);
}

void supla_device::set_device_channel_value(int SenderID, int ChannelID, const char value[SUPLA_CHANNELVALUE_SIZE]) {

	channels->set_device_channel_value(getSvrConn()->srpc(), SenderID, ChannelID, value);
}

bool supla_device::set_device_channel_char_value(int SenderID, int ChannelID, const char value) {

	return channels->set_device_channel_char_value(getSvrConn()->srpc(), SenderID, ChannelID, value);
}

bool supla_device::set_device_channel_rgbw_value(int SenderID, int ChannelID, int color, char color_brightness, char brightness) {

	return channels->set_device_channel_rgbw_value(getSvrConn()->srpc(), SenderID, ChannelID, color, color_brightness, brightness);
}

bool supla_device::channel_exists(int ChannelID) {
	return channels->channel_exists(ChannelID);
}

int supla_device::master_channel(int ChannelID) {
	return channels->master_channel(ChannelID);
}

int supla_device::slave_channel(int ChannelID) {
	return channels->slave_channel(ChannelID);
}

bool supla_device::get_channel_double_value(int ChannelID, double *Value) {
	return channels->get_channel_double_value(ChannelID, Value);
}

bool supla_device::get_channel_temperature_value(int ChannelID, double *Value) {
	return channels->get_channel_temperature_value(ChannelID, Value);
}

bool supla_device::get_channel_humidity_value(int ChannelID, double *Value) {
	return channels->get_channel_humidity_value(ChannelID, Value);
}

void supla_device::get_temp_and_humidity(void *tarr) {
	channels->get_temp_and_humidity(tarr);
}

bool supla_device::get_channel_char_value(int ChannelID, char *Value) {
	return channels->get_channel_char_value(ChannelID, Value);
}

bool supla_device::get_channel_rgbw_value(int ChannelID, int *color, char *color_brightness, char *brightness) {
	return channels->get_channel_rgbw_value(ChannelID, color, color_brightness, brightness);
}

void supla_device::get_firmware_update_url(void) {

	TSD_FirmwareUpdate_UrlResult result;
	memset(&result, 0, sizeof(TSD_FirmwareUpdate_UrlResult));

	database *db = new database();

	if ( db->connect() == true )
		db->get_device_firmware_update_url(getID(), &result);

	delete db;

	srpc_sd_async_get_firmware_update_url_result(getSvrConn()->srpc(), &result);
}
