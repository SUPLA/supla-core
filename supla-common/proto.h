/*
 ============================================================================
 Name        : proto.h
 Author      : Przemyslaw Zygmunt przemek@supla.org
 Version     : 1.3
 Copyright   : 2015-2016 GPLv2
 ============================================================================
 */

#ifndef supla_proto_H_
#define supla_proto_H_

#ifdef __AVR__

	#ifndef _TIMEVAL_DEFINED
		#define _TIMEVAL_DEFINED
		
		typedef long suseconds_t;
		#define	_TIME_T_	long
		typedef _TIME_T_ time_t;
		
		struct timeval {
		  time_t      tv_sec[2];
		  suseconds_t tv_usec[2];
		};
    #endif
    #define _supla_int_t long

#else
	#include <sys/time.h>
    #define _supla_int_t int
#endif 

#ifdef __cplusplus
extern "C" {
#endif

// DCS - device/client -> server
// SDC - server -> device/client
// DS  - device -> server
// SD  - server -> device
// CS  - client -> server
// SC  - server -> client

#define SUPLA_PROTO_VERSION                 4
#define SUPLA_PROTO_VERSION_MIN             1
#define SUPLA_TAG_SIZE                      5
#ifdef __AVR__
	#define SUPLA_MAX_DATA_SIZE                 512
#else
	#define SUPLA_MAX_DATA_SIZE                 10240
#endif
#define SUPLA_RC_MAX_DEV_COUNT              50
#define SUPLA_SOFTVER_MAXSIZE               21

#define SUPLA_GUID_SIZE                     16
#define SUPLA_GUID_HEXSIZE                  33
#define SUPLA_LOCATION_PWDHEX_MAXSIZE       65
#define SUPLA_LOCATION_PWD_MAXSIZE          33
#define SUPLA_ACCESSID_PWDHEX_MAXSIZE       65
#define SUPLA_ACCESSID_PWD_MAXSIZE          33
#define SUPLA_LOCATION_CAPTION_MAXSIZE      401
#define SUPLA_LOCATIONPACK_MAXSIZE          20
#define SUPLA_CHANNEL_CAPTION_MAXSIZE       401
#define SUPLA_CHANNELPACK_MAXSIZE           20

#define SUPLA_DCS_CALL_GETVERSION                         10
#define SUPLA_SDC_CALL_GETVERSION_RESULT                  20
#define SUPLA_SDC_CALL_VERSIONERROR                       30
#define SUPLA_DCS_CALL_PING_SERVER                        40
#define SUPLA_SDC_CALL_PING_SERVER_RESULT                 50
#define SUPLA_DS_CALL_REGISTER_DEVICE                     60
#define SUPLA_DS_CALL_REGISTER_DEVICE_B                   65 // ver. >= 2
#define SUPLA_SD_CALL_REGISTER_DEVICE_RESULT              70
#define SUPLA_CS_CALL_REGISTER_CLIENT                     80
#define SUPLA_SC_CALL_REGISTER_CLIENT_RESULT              90
#define SUPLA_DS_CALL_DEVICE_CHANNEL_VALUE_CHANGED        100
#define SUPLA_SD_CALL_CHANNEL_SET_VALUE                   110
#define SUPLA_DS_CALL_CHANNEL_SET_VALUE_RESULT            120
#define SUPLA_SC_CALL_LOCATION_UPDATE                     130
#define SUPLA_SC_CALL_LOCATIONPACK_UPDATE                 140
#define SUPLA_SC_CALL_CHANNEL_UPDATE                      150
#define SUPLA_SC_CALL_CHANNELPACK_UPDATE                  160
#define SUPLA_SC_CALL_CHANNEL_VALUE_UPDATE                170
#define SUPLA_CS_CALL_GET_NEXT                            180
#define SUPLA_SC_CALL_EVENT                               190
#define SUPLA_CS_CALL_CHANNEL_SET_VALUE                   200
#define SUPLA_CS_CALL_CHANNEL_SET_VALUE_B                 205 // ver. >= 3
#define SUPLA_DCS_CALL_SET_ACTIVITY_TIMEOUT               210 // ver. >= 2
#define SUPLA_SDC_CALL_SET_ACTIVITY_TIMEOUT_RESULT        220 // ver. >= 2

#define SUPLA_RESULT_DATA_TOO_LARGE         -4
#define SUPLA_RESULT_BUFFER_OVERFLOW        -3
#define SUPLA_RESULT_DATA_ERROR             -2
#define SUPLA_RESULT_VERSION_ERROR          -1
#define SUPLA_RESULT_FALSE                   0
#define SUPLA_RESULT_TRUE                    1

#define SUPLA_RESULTCODE_NONE                      0
#define SUPLA_RESULTCODE_UNSUPORTED                1
#define SUPLA_RESULTCODE_FALSE                     2
#define SUPLA_RESULTCODE_TRUE                      3
#define SUPLA_RESULTCODE_TEMPORARILY_UNAVAILABLE   4
#define SUPLA_RESULTCODE_BAD_CREDENTIALS           5
#define SUPLA_RESULTCODE_LOCATION_CONFLICT         6
#define SUPLA_RESULTCODE_CHANNEL_CONFLICT          7
#define SUPLA_RESULTCODE_DEVICE_DISABLED           8
#define SUPLA_RESULTCODE_ACCESSID_DISABLED         9
#define SUPLA_RESULTCODE_LOCATION_DISABLED         10
#define SUPLA_RESULTCODE_CLIENT_DISABLED           11
#define SUPLA_RESULTCODE_CLIENT_LIMITEXCEEDED      12
#define SUPLA_RESULTCODE_DEVICE_LIMITEXCEEDED      13
#define SUPLA_RESULTCODE_GUID_ERROR                14

#define SUPLA_DEVICE_NAME_MAXSIZE                  201
#define SUPLA_DEVICE_NAMEHEX_MAXSIZE               401
#define SUPLA_CLIENT_NAME_MAXSIZE                  201
#define SUPLA_CLIENT_NAMEHEX_MAXSIZE               401
#define SUPLA_SENDER_NAME_MAXSIZE                  201

#ifdef __AVR__
	#ifdef __AVR_ATmega2560__
		#define SUPLA_CHANNELMAXCOUNT                      16
	#else
		#define SUPLA_CHANNELMAXCOUNT                      1
	#endif
#else
#define SUPLA_CHANNELMAXCOUNT                      128
#endif

#define SUPLA_CHANNELVALUE_SIZE                    8

#define SUPLA_CHANNELTYPE_SENSORNO                 1000
#define SUPLA_CHANNELTYPE_SENSORNC                 1010  // ver. >= 4
#define SUPLA_CHANNELTYPE_CALLBUTTON               1500  // ver. >= 4
#define SUPLA_CHANNELTYPE_RELAYHFD4                2000
#define SUPLA_CHANNELTYPE_RELAYG5LA1A              2010
#define SUPLA_CHANNELTYPE_2XRELAYG5LA1A            2020
#define SUPLA_CHANNELTYPE_RELAY                    2900
#define SUPLA_CHANNELTYPE_THERMOMETERDS18B20       3000
#define SUPLA_CHANNELTYPE_DHT11                    3010  // ver. >= 4
#define SUPLA_CHANNELTYPE_DHT22                    3020  // ver. >= 4
#define SUPLA_CHANNELTYPE_AM2302                   3030  // ver. >= 4
#define SUPLA_CHANNELTYPE_DIMMER                   4000  // ver. >= 4
#define SUPLA_CHANNELTYPE_RGBLEDCONTROLLER         4010  // ver. >= 4
#define SUPLA_CHANNELTYPE_DIMMERANDRGBLED          4020  // ver. >= 4

#define SUPLA_CHANNELDRIVER_MCP23008               2

#define SUPLA_CHANNELFNC_NONE                               0
#define SUPLA_CHANNELFNC_CONTROLLINGTHEGATEWAYLOCK         10
#define SUPLA_CHANNELFNC_CONTROLLINGTHEGATE                20
#define SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR          30
#define SUPLA_CHANNELFNC_THERMOMETER                       40
#define SUPLA_CHANNELFNC_HUMIDITY                          42
#define SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE            45
#define SUPLA_CHANNELFNC_OPENINGSENSOR_GATEWAY             50
#define SUPLA_CHANNELFNC_OPENINGSENSOR_GATE                60
#define SUPLA_CHANNELFNC_OPENINGSENSOR_GARAGEDOOR          70
#define SUPLA_CHANNELFNC_NOLIQUIDSENSOR                    80
#define SUPLA_CHANNELFNC_CONTROLLINGTHEDOORLOCK            90
#define SUPLA_CHANNELFNC_OPENINGSENSOR_DOOR               100
#define SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER      110
#define SUPLA_CHANNELFNC_OPENINGSENSOR_ROLLERSHUTTER      120
#define SUPLA_CHANNELFNC_POWERSWITCH                      130
#define SUPLA_CHANNELFNC_LIGHTSWITCH                      140
#define SUPLA_CHANNELFNC_RING                             150
#define SUPLA_CHANNELFNC_ALARM                            160
#define SUPLA_CHANNELFNC_NOTIFICATION                     170
#define SUPLA_CHANNELFNC_DIMMER                           180
#define SUPLA_CHANNELFNC_RGBLIGHTING                      190
#define SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING             200

#define SUPLA_BIT_RELAYFUNC_CONTROLLINGTHEGATEWAYLOCK         0x0001
#define SUPLA_BIT_RELAYFUNC_CONTROLLINGTHEGATE                0x0002
#define SUPLA_BIT_RELAYFUNC_CONTROLLINGTHEGARAGEDOOR          0x0004
#define SUPLA_BIT_RELAYFUNC_CONTROLLINGTHEDOORLOCK            0x0008
#define SUPLA_BIT_RELAYFUNC_CONTROLLINGTHEROLLERSHUTTER       0x0010
#define SUPLA_BIT_RELAYFUNC_POWERSWITCH                       0x0020
#define SUPLA_BIT_RELAYFUNC_LIGHTSWITCH                       0x0040

#define SUPLA_EVENT_CONTROLLINGTHEGATEWAYLOCK               10
#define SUPLA_EVENT_CONTROLLINGTHEGATE                      20
#define SUPLA_EVENT_CONTROLLINGTHEGARAGEDOOR                30
#define SUPLA_EVENT_CONTROLLINGTHEDOORLOCK                  40
#define SUPLA_EVENT_CONTROLLINGTHEROLLERSHUTTER             50
#define SUPLA_EVENT_POWERONOFF                              60
#define SUPLA_EVENT_LIGHTONOFF                              70

#pragma pack(push, 1)


typedef struct {

  char tag[SUPLA_TAG_SIZE];
  unsigned char version;
  unsigned _supla_int_t rr_id; // Request/Response ID
  unsigned _supla_int_t call_type;
  unsigned _supla_int_t data_size;
  char data[SUPLA_MAX_DATA_SIZE]; // Last variable in struct!

}TSuplaDataPacket;

typedef struct {
	// server -> device|client

	unsigned char proto_version_min;
	unsigned char proto_version;
	char SoftVer[SUPLA_SOFTVER_MAXSIZE];

}TSDC_SuplaGetVersionResult;


typedef struct {
	// server -> device|client

	unsigned char server_version_min;
	unsigned char server_version;

}TSDC_SuplaVersionError;

typedef struct {

	// device|client -> server

	struct timeval now;

}TDCS_SuplaPingServer;

typedef struct {

	// server -> device|client

	struct timeval now;

}TSDC_SuplaPingServerResult;

typedef struct {
	// device|client -> server

	unsigned char activity_timeout;

}TDCS_SuplaSetActivityTimeout;

typedef struct {
	// server -> device|client

	unsigned char activity_timeout;
	unsigned char min;
	unsigned char max;

}TSDC_SuplaSetActivityTimeoutResult;

typedef struct {
	char value[SUPLA_CHANNELVALUE_SIZE];
	char sub_value[SUPLA_CHANNELVALUE_SIZE]; // For example sensor value
}TSuplaChannelValue;


typedef struct {
	// server -> client
	char EOL; // End Of List

	_supla_int_t Id;
	unsigned _supla_int_t CaptionSize; // including the terminating null byte ('\0')
	char Caption[SUPLA_LOCATION_CAPTION_MAXSIZE]; // Last variable in struct!

}TSC_SuplaLocation;

typedef struct {
	// server -> client

	_supla_int_t count;
	_supla_int_t total_left;
	TSC_SuplaLocation locations[SUPLA_LOCATIONPACK_MAXSIZE]; // Last variable in struct!

}TSC_SuplaLocationPack;

typedef struct {
	// device -> server

	unsigned char Number;
	_supla_int_t Type;

	char value[SUPLA_CHANNELVALUE_SIZE];

}TDS_SuplaDeviceChannel;

typedef struct {
	// device -> server

	_supla_int_t LocationID;
	char LocationPWD[SUPLA_LOCATION_PWD_MAXSIZE]; // UTF8

	char GUID[SUPLA_GUID_SIZE];
	char Name[SUPLA_DEVICE_NAME_MAXSIZE]; // UTF8
	char SoftVer[SUPLA_SOFTVER_MAXSIZE];

	unsigned char channel_count;
	TDS_SuplaDeviceChannel channels[SUPLA_CHANNELMAXCOUNT]; // Last variable in struct!

}TDS_SuplaRegisterDevice;

typedef struct {
	// device -> server

	unsigned char Number;
	_supla_int_t Type;

	_supla_int_t FuncList;
	_supla_int_t Default;

	char value[SUPLA_CHANNELVALUE_SIZE];

}TDS_SuplaDeviceChannel_B; // ver. >= 2


typedef struct {
	// device -> server

	_supla_int_t LocationID;
	char LocationPWD[SUPLA_LOCATION_PWD_MAXSIZE]; // UTF8

	char GUID[SUPLA_GUID_SIZE];
	char Name[SUPLA_DEVICE_NAME_MAXSIZE]; // UTF8
	char SoftVer[SUPLA_SOFTVER_MAXSIZE];

	unsigned char channel_count;
	TDS_SuplaDeviceChannel_B channels[SUPLA_CHANNELMAXCOUNT]; // Last variable in struct!

}TDS_SuplaRegisterDevice_B; // ver. >= 2

typedef struct {
	// server -> device

	_supla_int_t result_code;
	unsigned char activity_timeout;
	unsigned char version;
	unsigned char version_min;

}TSD_SuplaRegisterDeviceResult;

typedef struct {
	// device -> server

	unsigned char ChannelNumber;
	char value[SUPLA_CHANNELVALUE_SIZE];

}TDS_SuplaDeviceChannelValue;

typedef struct {
	// server -> device
	_supla_int_t SenderID;
	unsigned char ChannelNumber;
	unsigned _supla_int_t DurationMS;

	char value[SUPLA_CHANNELVALUE_SIZE];

}TSD_SuplaChannelNewValue;

typedef struct {
	// device -> server
	unsigned char ChannelNumber;
	_supla_int_t SenderID;
	char Success;

}TDS_SuplaChannelNewValueResult;

typedef struct {
	// server -> client

	char EOL; // End Of List
	_supla_int_t Id;
	char online;

	TSuplaChannelValue value;

}TSC_SuplaChannelValue;


typedef struct {
	// server -> client
	char EOL; // End Of List

	_supla_int_t Id;
	_supla_int_t LocationID;
	_supla_int_t Func;
	char online;

	TSuplaChannelValue value;

	unsigned _supla_int_t CaptionSize; // including the terminating null byte ('\0')
	char Caption[SUPLA_CHANNEL_CAPTION_MAXSIZE]; // Last variable in struct!

}TSC_SuplaChannel;


typedef struct {
	// server -> client

	_supla_int_t count;
	_supla_int_t total_left;
	TSC_SuplaChannel channels[SUPLA_CHANNELPACK_MAXSIZE]; // Last variable in struct!

}TSC_SuplaChannelPack;


typedef struct {
	// client -> server

	_supla_int_t AccessID;
	char AccessIDpwd[SUPLA_ACCESSID_PWD_MAXSIZE]; // UTF8

	char GUID[SUPLA_GUID_SIZE];
	char Name[SUPLA_CLIENT_NAME_MAXSIZE]; // UTF8
	char SoftVer[SUPLA_SOFTVER_MAXSIZE];


}TCS_SuplaRegisterClient;

typedef struct {
	// server -> client

	_supla_int_t result_code;
	_supla_int_t ClientID;
	_supla_int_t LocationCount;
	_supla_int_t ChannelCount;
	unsigned char activity_timeout;
	unsigned char version;
	unsigned char version_min;

}TSC_SuplaRegisterClientResult;


typedef struct {
	// client -> server
	unsigned char ChannelId;
	char value[SUPLA_CHANNELVALUE_SIZE];

}TCS_SuplaChannelNewValue; // Deprecated

typedef struct {
	// client -> server
	_supla_int_t ChannelId;
	char value[SUPLA_CHANNELVALUE_SIZE];

}TCS_SuplaChannelNewValue_B;

typedef struct {
    // server -> client
	_supla_int_t Event;
	_supla_int_t ChannelID;
	unsigned _supla_int_t DurationMS;

	_supla_int_t SenderID;
	unsigned _supla_int_t SenderNameSize; // including the terminating null byte ('\0')
	char SenderName[SUPLA_SENDER_NAME_MAXSIZE]; // Last variable in struct! | UTF8

}TSC_SuplaEvent;

#pragma pack(pop)

void *sproto_init(void);
void sproto_free(void *spd_ptr);

char sproto_in_buffer_append(void *spd_ptr, char *data, unsigned _supla_int_t data_size);
char sproto_out_buffer_append(void *spd_ptr, TSuplaDataPacket *sdp);

char sproto_pop_in_sdp(void *spd_ptr, TSuplaDataPacket *sdp);
unsigned _supla_int_t sproto_pop_out_data(void *spd_ptr, char *buffer, unsigned _supla_int_t buffer_size);
char sproto_out_dataexists(void *spd_ptr);

unsigned char sproto_get_version(void *spd_ptr);
void sproto_set_version(void *spd_ptr, unsigned char version);
void sproto_sdp_init(void *spd_ptr, TSuplaDataPacket *sdp);
char sproto_set_data(TSuplaDataPacket *sdp, char *data, unsigned _supla_int_t data_size, unsigned _supla_int_t call_type);
TSuplaDataPacket* sproto_sdp_malloc(void *spd_ptr);
void sproto_sdp_free(TSuplaDataPacket* sdp);

void sproto_log_summary(void *spd_ptr);
void sproto_buffer_dump(void *spd_ptr, unsigned char in);

#ifdef __cplusplus
}
#endif

#endif /* supla_proto_H_ */
