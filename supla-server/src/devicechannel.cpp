/*
 ============================================================================
 Name        : devicechannel.cpp
 Author      : Przemyslaw Zygmunt p.zygmunt@acsoftware.pl [AC SOFTWARE]
 Version     : 1.0
 Copyright   : GPLv2
 ============================================================================
 */

#include <string.h>
#include <stdlib.h>

#include "devicechannel.h"
#include "safearray.h"
#include "db.h"
#include "log.h"
#include "srpc.h"

char supla_channel_tarr_clean(void *ptr) {
	delete (supla_channel_temphum*)ptr;
	return 1;
}

supla_channel_temphum::supla_channel_temphum(char TempAndHumidity, int ChannelId, double Temperature, double Humidity) {

	this->ChannelId = ChannelId;
	this->TempAndHumidity = TempAndHumidity;
	this->Temperature = Temperature;
	this->Humidity = Humidity;
}

int supla_channel_temphum::getChannelId(void) {
	return ChannelId;
}

char supla_channel_temphum::isTempAndHumidity(void) {
	return TempAndHumidity;
}

double supla_channel_temphum::getTemperature(void) {
	return Temperature;
}

double supla_channel_temphum::getHumidity(void) {
	return Humidity;
}


void supla_channel_temphum::free(void *tarr) {

	safe_array_clean(tarr, supla_channel_tarr_clean);
	safe_array_free(tarr);
}

//-----------------------------------------------------

supla_device_channel::supla_device_channel(int Id, int Number, int Type, int Func, int Param1, int Param2, int Param3) {

	this->Id = Id;
	this->Number = Number;
	this->Type = Type;
	this->Func = Func;
	this->Param1 = Param1;
	this->Param2 = Param2;
	this->Param3 = Param3;

	memset(this->value, 0, SUPLA_CHANNELVALUE_SIZE);

	if ( Type == SUPLA_CHANNELTYPE_SENSORNC ) {
		this->value[0] = 1;
	}
}


int supla_device_channel::getId(void) {
	return Id;
}

int supla_device_channel::getNumber(void) {
	return Number;
}

int supla_device_channel::getFunc(void) {
	return Func;
}

int supla_device_channel::getType(void) {
	return Type;
}

void supla_device_channel::getValue(char value[SUPLA_CHANNELVALUE_SIZE]) {

	memcpy(value, this->value, SUPLA_CHANNELVALUE_SIZE);

}

void supla_device_channel::getDouble(double *Value) {

	if ( Value == NULL ) return;

	switch(Type) {
	case SUPLA_CHANNELTYPE_SENSORNO:
	case SUPLA_CHANNELTYPE_SENSORNC:
		*Value = this->value[0] == 1 ? 1 : 0;
		break;
	case SUPLA_CHANNELTYPE_THERMOMETERDS18B20:
	case SUPLA_CHANNELTYPE_HCSR04:
		memcpy(Value, this->value, sizeof(double));
		break;
	default:
		*Value = 0;
	}

}

void supla_device_channel::getChar(char *Value) {

	if ( Value == NULL ) return;
	*Value = this->value[0];

}

void supla_device_channel::setValue(char value[SUPLA_CHANNELVALUE_SIZE]) {

	memcpy(this->value, value, SUPLA_CHANNELVALUE_SIZE);

    if ( Type == SUPLA_CHANNELTYPE_SENSORNC ) {
    	this->value[0] = this->value[0] == 0 ? 1 : 0;
    }

}

bool supla_device_channel::isValueWritable(void) {

	switch(Func) {
	case  SUPLA_CHANNELFNC_CONTROLLINGTHEGATEWAYLOCK:
	case  SUPLA_CHANNELFNC_CONTROLLINGTHEGATE:
	case  SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR:
	case  SUPLA_CHANNELFNC_CONTROLLINGTHEDOORLOCK:
	case  SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER:
	case  SUPLA_CHANNELFNC_POWERSWITCH:
	case  SUPLA_CHANNELFNC_LIGHTSWITCH:
	case  SUPLA_CHANNELFNC_DIMMER:
	case  SUPLA_CHANNELFNC_RGBLIGHTING:
	case  SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING:
		return 1;

		break;
	}

	return 0;
}

unsigned int supla_device_channel::getValueDuration(void) {

	switch(Func) {
	case  SUPLA_CHANNELFNC_CONTROLLINGTHEGATEWAYLOCK:
	case  SUPLA_CHANNELFNC_CONTROLLINGTHEGATE:
	case  SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR:
	case  SUPLA_CHANNELFNC_CONTROLLINGTHEDOORLOCK:

		return Param1;

		break;
	}

	return 0;
}

int supla_device_channel::slave_channel(void) {


	switch(Func) {
	case  SUPLA_CHANNELFNC_CONTROLLINGTHEGATEWAYLOCK:
	case  SUPLA_CHANNELFNC_CONTROLLINGTHEGATE:
	case  SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR:
	case  SUPLA_CHANNELFNC_CONTROLLINGTHEDOORLOCK:
	case  SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER:

		return Param2;

		break;
	}

	return 0;
}

int supla_device_channel::master_channel(void) {


	switch(Func) {
	case  SUPLA_CHANNELFNC_OPENINGSENSOR_GATEWAY:
	case  SUPLA_CHANNELFNC_OPENINGSENSOR_GATE:
	case  SUPLA_CHANNELFNC_OPENINGSENSOR_GARAGEDOOR:
	case  SUPLA_CHANNELFNC_OPENINGSENSOR_DOOR:
	case  SUPLA_CHANNELFNC_OPENINGSENSOR_ROLLERSHUTTER:

		return Param1;

		break;
	}

	return 0;
}

supla_channel_temphum *supla_device_channel::getTempHum(void) {

	double temp;


    if ( getType() == SUPLA_CHANNELTYPE_THERMOMETERDS18B20
	 	&& getFunc() == SUPLA_CHANNELFNC_THERMOMETER  ) {

			getDouble(&temp);

			if ( temp > -273 && temp <= 1000 ) {
				return new supla_channel_temphum(0, getId(), temp, 0);
			}

 	} else if ( ( getType() == SUPLA_CHANNELTYPE_DHT11
		 			 || getType() == SUPLA_CHANNELTYPE_DHT22
					 || getType() == SUPLA_CHANNELTYPE_AM2302 )
			 	&& ( getFunc() == SUPLA_CHANNELFNC_THERMOMETER
					 || getFunc() == SUPLA_CHANNELFNC_HUMIDITY
					 || getFunc() == SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE ) ) {

			int n;
			char value[SUPLA_CHANNELVALUE_SIZE];
			double humidity;

			getValue(value);
			memcpy(&n, value, 4);
			temp = n/1000.00;

			memcpy(&n, &value[4], 4);
			humidity = n/1000.00;

			if ( temp > -273
				 && temp <= 1000
				 && humidity >= 0
				 && humidity <= 100 ) {

				return new supla_channel_temphum(1, getId(), temp, humidity);
			}
	}

	return NULL;
}

// ---------------------------------------------
// ---------------------------------------------
// ---------------------------------------------

supla_device_channels::supla_device_channels() {

	arr = safe_array_init();

}

supla_device_channels::~supla_device_channels() {
	arr_clean();
	safe_array_free(arr);
}

char supla_device_channels::arr_findcmp(void *ptr, void *id) {
	return ((supla_device_channel*)ptr)->getId() == *((int*)id) ? 1 : 0;
}

char supla_device_channels::arr_findncmp(void *ptr, void *n) {
	return ((supla_device_channel*)ptr)->getNumber() == *((int*)n) ? 1 : 0;
}

char supla_device_channels::arr_delcnd(void *ptr) {

	delete (supla_device_channel*)ptr;
	return 1;
}

void supla_device_channels::arr_clean(void) {

	safe_array_lock(arr);
	safe_array_clean(arr, arr_delcnd);
	safe_array_unlock(arr);

}

supla_device_channel *supla_device_channels::find_channel(int Id) {

	return (supla_device_channel *)safe_array_findcnd(arr, arr_findcmp, &Id);

}

supla_device_channel *supla_device_channels::find_channel_by_number(int Number) {

	return (supla_device_channel *)safe_array_findcnd(arr, arr_findncmp, &Number);

}

void supla_device_channels::add_channel(int Id, int Number, int Type, int Func, int Param1, int Param2, int Param3) {

	safe_array_lock(arr);

	if ( find_channel(Id) == 0 ) {
		safe_array_add(arr, new supla_device_channel(Id, Number, Type, Func, Param1, Param2, Param3));
	}

	safe_array_unlock(arr);

}

void supla_device_channels::load(int DeviceID) {

	database *db = new database();

	if ( db->connect() == true ) {

		safe_array_lock(arr);
		arr_clean();

		db->get_device_channels(DeviceID, this);

		safe_array_unlock(arr);
	}

	delete db;

}

bool supla_device_channels::get_channel_value(int ChannelID, char value[SUPLA_CHANNELVALUE_SIZE]) {

	bool result = false;

	if ( ChannelID ) {

		safe_array_lock(arr);
		supla_device_channel *channel = find_channel(ChannelID);

		if ( channel ) {
			channel->getValue(value);
			result = true;
		}


		safe_array_unlock(arr);

	}

	return result;

}

bool supla_device_channels::get_channel_double_value(int ChannelID, double *Value) {

	bool result = false;

	if ( ChannelID ) {

		safe_array_lock(arr);
		supla_device_channel *channel = find_channel(ChannelID);

		if ( channel ) {
			channel->getDouble(Value);
			result = true;
		}


		safe_array_unlock(arr);

	}

	return result;

}

supla_channel_temphum* supla_device_channels::get_channel_temp_and_humidity_value(int ChannelID) {

	supla_channel_temphum *result = NULL;

	if ( ChannelID ) {

		safe_array_lock(arr);
		supla_device_channel *channel = find_channel(ChannelID);

		if ( channel ) {
			result = channel->getTempHum();
		}

		safe_array_unlock(arr);

	}

	return result;

}

bool supla_device_channels::get_channel_temperature_value(int ChannelID, double *Value) {

	supla_channel_temphum *result = get_channel_temp_and_humidity_value(ChannelID);
	if ( result ) {
		*Value = result->getTemperature();
		delete result;
		return true;
	}

	return false;
}

bool supla_device_channels::get_channel_humidity_value(int ChannelID, double *Value) {

	supla_channel_temphum *result = get_channel_temp_and_humidity_value(ChannelID);
	if ( result ) {
		if ( result->isTempAndHumidity() == 1 ) {

			*Value = result->getHumidity();
			delete result;
			return true;

		} else {
			delete result;
		}

	}

	return false;
}

bool supla_device_channels::get_channel_char_value(int ChannelID, char *Value) {

	bool result = false;

	if ( ChannelID ) {

		safe_array_lock(arr);
		supla_device_channel *channel = find_channel(ChannelID);

		if ( channel ) {
			channel->getChar(Value);
			result = true;
		}


		safe_array_unlock(arr);

	}

	return result;

}

void supla_device_channels::set_channel_value(int ChannelID, char value[SUPLA_CHANNELVALUE_SIZE]) {

	if ( ChannelID == 0 )
		return;

	safe_array_lock(arr);

	supla_device_channel *channel = find_channel(ChannelID);

	if ( channel )
		channel->setValue(value);

	safe_array_unlock(arr);

}

unsigned int supla_device_channels::get_channel_value_duration(int ChannelID) {

	if ( ChannelID == 0 )
		return 0;

	int Duration = 0;

	safe_array_lock(arr);

	supla_device_channel *channel = find_channel(ChannelID);

	if ( channel )
		Duration = channel->getValueDuration();

	safe_array_unlock(arr);

	return Duration;

}

int supla_device_channels::get_channel_func(int ChannelID) {

	if ( ChannelID == 0 )
		return 0;

	int Func = 0;

	safe_array_lock(arr);

	supla_device_channel *channel = find_channel(ChannelID);

	if ( channel )
		Func = channel->getFunc();

	safe_array_unlock(arr);

	return Func;

}

int supla_device_channels::ms_channel(int ChannelID, bool Master) {

	if ( ChannelID == 0 )
		return 0;

	int SubChannelId = 0;

	safe_array_lock(arr);

	supla_device_channel *channel = find_channel(ChannelID);

	if ( channel )
		SubChannelId = Master ? channel->master_channel() : channel->slave_channel();

	safe_array_unlock(arr);

	return SubChannelId;
}

int supla_device_channels::master_channel(int ChannelID) {
	return ms_channel(ChannelID, true);
}

int supla_device_channels::slave_channel(int ChannelID) {
	return ms_channel(ChannelID, false);
}

bool supla_device_channels::channel_exists(int ChannelID) {
	bool result = false;

	safe_array_lock(arr);

	if ( find_channel(ChannelID) != NULL )
		result = true;

	safe_array_unlock(arr);

	return result;
}

void supla_device_channels::set_channels_value(TDS_SuplaDeviceChannel_B *schannel, int count) {

	for(int a=0;a<count;a++)
		set_channel_value(get_channel_id(schannel[a].Number), schannel[a].value);

}

int supla_device_channels::get_channel_id(unsigned char ChannelNumber) {

	int result = 0;

	safe_array_lock(arr);

	supla_device_channel *channel = find_channel_by_number(ChannelNumber);

	if ( channel )
		result = channel->getId();

	safe_array_unlock(arr);

	return result;
}

void supla_device_channels::set_device_channel_value(void *srpc, int SenderID, int ChannelID, const char value[SUPLA_CHANNELVALUE_SIZE])  {

	safe_array_lock(arr);

	supla_device_channel *channel = find_channel(ChannelID);

	if ( channel
		 && channel->isValueWritable() ) {

		TSD_SuplaChannelNewValue s;
		s.ChannelNumber = channel->getNumber();
		s.DurationMS = channel->getValueDuration();
		s.SenderID = SenderID;
		memcpy(s.value, value, SUPLA_CHANNELVALUE_SIZE);

		srpc_sd_async_set_channel_value(srpc, &s);
	}

	safe_array_unlock(arr);

}

void supla_device_channels::get_temp_and_humidity(void *tarr) {

	int a;
	safe_array_lock(arr);

	for(a=0;a<safe_array_count(arr);a++) {

		supla_device_channel *channel = (supla_device_channel *)safe_array_get(arr, a);

		if ( channel != NULL ) {

			   supla_channel_temphum *temphum = channel->getTempHum();

			   if ( temphum != NULL )
				   safe_array_add(tarr, temphum);
		}

	}

	safe_array_unlock(arr);

}

