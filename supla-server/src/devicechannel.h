/*
 ============================================================================
 Name        : devicechannel.h
 Author      : Przemyslaw Zygmunt p.zygmunt@acsoftware.pl [AC SOFTWARE]
 Version     : 1.0
 Copyright   : GPLv2
 ============================================================================
 */

#ifndef DEVICECHANNEL_H_
#define DEVICECHANNEL_H_

#include "proto.h"

class supla_channel_temphum {

private:
	int ChannelId;
	char TempAndHumidity;
	double Temperature;
	double Humidity;
public:
	supla_channel_temphum(char TempAndHumidity, int ChannelId, double Temperature, double Humidity);

	int getChannelId(void);
	char isTempAndHumidity(void);
	double getTemperature(void);
	double getHumidity(void);

	static void free(void *tarr);
};

class supla_device_channel {
private:

	int Id;
	unsigned char Number;
    int Type;
    int Func;
    int Param1;
    int Param2;
    int Param3;

    char value[8];

public:
	supla_device_channel(int Id, int Number, int Type, int Func, int Param1, int Param2, int Param3);
	int getId(void);
	int getNumber(void);
	int getFunc(void);
	int getType(void);
	bool isValueWritable(void);
	unsigned int getValueDuration(void);
	void getValue(char value[SUPLA_CHANNELVALUE_SIZE]);
	void setValue(char value[SUPLA_CHANNELVALUE_SIZE]);
	void getDouble(double *Value);
	void getChar(char *Value);
	int master_channel(void);
	int slave_channel(void);
	supla_channel_temphum *getTempHum(void);
};

class supla_device_channels {
private:
	void *arr;

	static char arr_findcmp(void *ptr, void *id);
	static char arr_findncmp(void *ptr, void *n);
	static char arr_delcnd(void *ptr);
	void arr_clean(void);

	supla_device_channel *find_channel(int Id) ;
	supla_device_channel *find_channel_by_number(int Number);

	int ms_channel(int ChannelID, bool Master);

public:

	supla_device_channels();
	virtual ~supla_device_channels();
	void add_channel(int Id, int Number, int Type, int Func, int Param1, int Param2, int Param3);
	bool get_channel_value(int ChannelID, char value[SUPLA_CHANNELVALUE_SIZE]);
	bool get_channel_double_value(int ChannelID, double *Value);
	supla_channel_temphum* get_channel_temp_and_humidity_value(int ChannelID);
	bool get_channel_temperature_value(int ChannelID, double *Value);
	bool get_channel_humidity_value(int ChannelID, double *Value);
	bool get_channel_char_value(int ChannelID, char *Value);
	unsigned int get_channel_value_duration(int ChannelID);
	int get_channel_func(int ChannelID);
	void set_channel_value(int ChannelID, char value[SUPLA_CHANNELVALUE_SIZE]);
	void set_channels_value(TDS_SuplaDeviceChannel_B *schannel, int count);

	void set_device_channel_value(void *srpc, int SenderID, int ChannelID, const char value[SUPLA_CHANNELVALUE_SIZE]) ;

	int master_channel(int ChannelID);
	int slave_channel(int ChannelID);
	int get_channel_id(unsigned char ChannelNumber);
    bool channel_exists(int ChannelID);
	void load(int DeviceID);

	void get_temp_and_humidity(void *tarr);
};

#endif /* DEVICECHANNEL_H_ */
