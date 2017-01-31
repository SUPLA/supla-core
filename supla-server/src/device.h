/*
 ============================================================================
 Name        : device.h
 Author      : Przemyslaw Zygmunt p.zygmunt@acsoftware.pl [AC SOFTWARE]
 Version     : 1.0
 Copyright   : GPLv2
 ============================================================================
 */

#ifndef DEVICE_H_
#define DEVICE_H_

#include "cdcommon.h"
#include "devicechannel.h"

class supla_user;
class supla_device : public cdcommon {
protected:

	supla_device_channels *channels;

	void load_config(void);
	static char channels_clean_cnd(void *channel);

public:
    supla_device(serverconnection *svrconn);
	char register_device(TDS_SuplaRegisterDevice_B *register_device, unsigned char proto_version);
	virtual ~supla_device();

	bool get_channel_value(int ChannelID, char value[SUPLA_CHANNELVALUE_SIZE]);
	void set_device_channel_value(int SenderID, int ChannelID, const char value[SUPLA_CHANNELVALUE_SIZE]);

	bool channel_exists(int ChannelID);
	void on_device_channel_value_changed(TDS_SuplaDeviceChannelValue *value);
	void on_channel_set_value_result(TDS_SuplaChannelNewValueResult *result) ;
	int master_channel(int ChannelID);
	int slave_channel(int ChannelID);
	bool get_channel_double_value(int ChannelID, double *Value);
	bool get_channel_temperature_value(int ChannelID, double *Value);
	bool get_channel_humidity_value(int ChannelID, double *Value);
	void get_temp_and_humidity(void *tarr);
	bool get_channel_char_value(int ChannelID, char *Value);
	bool get_channel_rgbw_value(int ChannelID, int *color, char *color_brightness, char *brightness);
};

#endif /* DEVICE_H_ */
