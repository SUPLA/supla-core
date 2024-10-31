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

#ifndef supla_proto_H_
#define supla_proto_H_

#ifdef _WIN32
// *** WINDOWS ***

#include <WinSock2.h>
#define _supla_int_t int
#define _supla_int16_t short
#define _supla_int64_t __int64
#define _supla_timeval timeval

#elif defined(__AVR__)
// *** ARDUINO MEGA ***

#define SPROTO_WITHOUT_OUT_BUFFER

struct _supla_timeval {
  long tv_sec[2];
  long tv_usec[2];
};

#define timeval _supla_timeval

#define _supla_int16_t int
#define _supla_int_t long
#define _supla_int64_t long long

#elif defined(ESP8266) || defined(ESP32) || defined(ESP_PLATFORM)
// *** Espressif NONOS SDK for ESP8266 OR ARDUINO WITH ESP8266 or ESP32 ***
// *** ESP-IDF, ESP8266 RTOS SDK ***
#ifndef ESP_PLATFORM
#ifndef ARDUINO
#include <mem.h>
#define PROTO_ICACHE_FLASH ICACHE_FLASH_ATTR
#endif /*ARDUINO*/
#endif /*ESP_PLATFORM*/

#if defined(ARDUINO) || defined(ESP_PLATFORM)
// *** Only ARDUINO ESPxxx and ESP-IDS, RTOS SDK (NONOS is excluded) ***
#define SPROTO_WITHOUT_OUT_BUFFER
#endif /*ARDUINO_ARCH_ESP8266*/

struct _supla_timeval {
  long long tv_sec;
  long long tv_usec;
};

#define _supla_int16_t short
#define _supla_int_t int
#define _supla_int64_t long long
#elif defined(__arm__)
// *** ARM - RPI? ***

struct _supla_timeval {
  long long tv_sec;
  long long tv_usec;
};

#include <sys/time.h>
#define _supla_int16_t short
#define _supla_int_t int
#define _supla_int64_t long long

#else /*__arm__*/
// *** OTHER? Linux, what else? ***
#include <sys/time.h>
#define _supla_int16_t short
#define _supla_int_t int
#define _supla_int64_t long long
// timeval for 64 bit system
#if __GNUC__
#if __x86_64__ || __ppc64__
#define _supla_timeval timeval
#else
// timeval for 32 bit system
struct _supla_timeval {
  long long tv_sec;
  long long tv_usec;
};

#endif /* __x86_64__ || __ppc64__*/
#endif /* __GNUC__*/
#endif

#ifndef PROTO_ICACHE_FLASH
#define PROTO_ICACHE_FLASH
#endif /*PROTO_ICACHE_FLASH*/

#ifdef __cplusplus
extern "C" {
#endif

#define SUPLA_TAG_SIZE 5
extern char sproto_tag[SUPLA_TAG_SIZE];

// DCS - device/client -> server
// SDC - server -> device/client
// DS  - device -> server
// SD  - server -> device
// CS  - client -> server
// SC  - server -> client

#define SUPLA_PROTO_VERSION 25
#define SUPLA_PROTO_VERSION_MIN 1

#if defined(ARDUINO_ARCH_AVR) || defined(ARDUINO) || defined(SUPLA_DEVICE)
// All Arduino IDE and SuplaDevice compilations
// Max packet size was reduced and register device is now send in chunks:
// first is header, then channel data
// SUPLA_MAX_DATA_SIZE should be bigger then calcfg, device config, channel
// config MAXSIZE. Otherwise sending will fail
#define SUPLA_MAX_DATA_SIZE 600  // Registration header without channels
#define USE_DEPRECATED_EMEV_V2   // Temporary. It will be removed.
#elif defined(ESP8266)
#define USE_DEPRECATED_EMEV_V2  // Temporary. It will be removed.
// supla-espressif-esp compilations
#define SUPLA_MAX_DATA_SIZE 1536
#else
#define SUPLA_MAX_DATA_SIZE 10240
#endif
#define SUPLA_RC_MAX_DEV_COUNT 50
#define SUPLA_SOFTVER_MAXSIZE 21

#define SUPLA_CAPTION_MAXSIZE 401

#define SUPLA_GUID_SIZE 16
#define SUPLA_GUID_HEXSIZE 33
#define SUPLA_LOCATION_PWD_MAXSIZE 33
#define SUPLA_ACCESSID_PWD_MAXSIZE 33
#define SUPLA_LOCATION_CAPTION_MAXSIZE SUPLA_CAPTION_MAXSIZE
#define SUPLA_LOCATIONPACK_MAXCOUNT 20
#define SUPLA_CHANNEL_CAPTION_MAXSIZE SUPLA_CAPTION_MAXSIZE
#define SUPLA_CHANNEL_GROUP_CAPTION_MAXSIZE SUPLA_CAPTION_MAXSIZE
#define SUPLA_CHANNELPACK_MAXCOUNT 20
#define SUPLA_URL_HOST_MAXSIZE 101
#define SUPLA_URL_PATH_MAXSIZE 101
#define SUPLA_SERVER_NAME_MAXSIZE 65
#define SUPLA_EMAIL_MAXSIZE 256                     // ver. >= 7
#define SUPLA_PASSWORD_MAXSIZE 64                   // ver. >= 10
#define SUPLA_AUTHKEY_SIZE 16                       // ver. >= 7
#define SUPLA_AUTHKEY_HEXSIZE 33                    // ver. >= 7
#define SUPLA_OAUTH_TOKEN_MAXSIZE 256               // ver. >= 10
#define SUPLA_CHANNELGROUP_PACK_MAXCOUNT 20         // ver. >= 9
#define SUPLA_CHANNELGROUP_CAPTION_MAXSIZE 401      // ver. >= 9
#define SUPLA_CHANNELVALUE_PACK_MAXCOUNT 20         // ver. >= 9
#define SUPLA_CHANNELEXTENDEDVALUE_PACK_MAXCOUNT 5  // ver. >= 10
#define SUPLA_CHANNELEXTENDEDVALUE_PACK_MAXDATASIZE \
  (SUPLA_MAX_DATA_SIZE - 50)            // ver. >= 10
#define SUPLA_CALCFG_DATA_MAXSIZE 128   // ver. >= 10
#define SUPLA_TIMEZONE_MAXSIZE 51       // ver. >= 11
#define SUPLA_ACTION_PARAM_MAXSIZE 500  // ver. >= 18

#ifndef SUPLA_CHANNELGROUP_RELATION_PACK_MAXCOUNT
#define SUPLA_CHANNELGROUP_RELATION_PACK_MAXCOUNT 100  // ver. >= 9
#endif /*SUPLA_CHANNELGROUP_RELATION_PACK_MAXCOUNT*/

#define SUPLA_SCENE_CAPTION_MAXSIZE SUPLA_CAPTION_MAXSIZE  // ver. >= 18
#define SUPLA_SCENE_PACK_MAXCOUNT 20                       // ver. >= 18
#define SUPLA_SCENE_STATE_PACK_MAXCOUNT 20                 // ver. >= 18

#define SUPLA_CHANNEL_RELATION_PACK_MAXCOUNT 100  // ver. >= 21

#define SUPLA_DCS_CALL_GETVERSION 10
#define SUPLA_SDC_CALL_GETVERSION_RESULT 20
#define SUPLA_SDC_CALL_VERSIONERROR 30
#define SUPLA_DCS_CALL_PING_SERVER 40
#define SUPLA_SDC_CALL_PING_SERVER_RESULT 50
#define SUPLA_DS_CALL_REGISTER_DEVICE 60
#define SUPLA_DS_CALL_REGISTER_DEVICE_B 65  // ver. >= 2
#define SUPLA_DS_CALL_REGISTER_DEVICE_C 67  // ver. >= 6
#define SUPLA_DS_CALL_REGISTER_DEVICE_D 68  // ver. >= 7
#define SUPLA_DS_CALL_REGISTER_DEVICE_E 69  // ver. >= 10
#define SUPLA_SD_CALL_REGISTER_DEVICE_RESULT 70
#define SUPLA_SD_CALL_REGISTER_DEVICE_RESULT_B 71  // ver. >= 25
#define SUPLA_DS_CALL_REGISTER_DEVICE_F 75         // ver. >= 23
#define SUPLA_DS_CALL_REGISTER_DEVICE_G 76         // ver. >= 25
#define SUPLA_CS_CALL_REGISTER_CLIENT 80
#define SUPLA_CS_CALL_REGISTER_CLIENT_B 85  // ver. >= 6
#define SUPLA_CS_CALL_REGISTER_CLIENT_C 86  // ver. >= 7
#define SUPLA_CS_CALL_REGISTER_CLIENT_D 87  // ver. >= 12
#define SUPLA_SC_CALL_REGISTER_CLIENT_RESULT 90
#define SUPLA_SC_CALL_REGISTER_CLIENT_RESULT_B 92  // ver. >= 9
#define SUPLA_SC_CALL_REGISTER_CLIENT_RESULT_C 94  // ver. >= 17
#define SUPLA_SC_CALL_REGISTER_CLIENT_RESULT_D 96  // ver. >= 19
#define SUPLA_DS_CALL_DEVICE_CHANNEL_VALUE_CHANGED 100
#define SUPLA_DS_CALL_DEVICE_CHANNEL_VALUE_CHANGED_B 102        // ver. >= 12
#define SUPLA_DS_CALL_DEVICE_CHANNEL_VALUE_CHANGED_C 103        // ver. >= 12
#define SUPLA_DS_CALL_DEVICE_CHANNEL_EXTENDEDVALUE_CHANGED 105  // ver. >= 10
#define SUPLA_SD_CALL_CHANNEL_SET_VALUE 110
#define SUPLA_SD_CALL_CHANNELGROUP_SET_VALUE 115  // ver. >= 13
#define SUPLA_DS_CALL_CHANNEL_SET_VALUE_RESULT 120
#define SUPLA_SC_CALL_LOCATION_UPDATE 130
#define SUPLA_SC_CALL_LOCATIONPACK_UPDATE 140
#define SUPLA_SC_CALL_CHANNELPACK_UPDATE 160
#define SUPLA_SC_CALL_CHANNEL_VALUE_UPDATE 170
#define SUPLA_SC_CALL_CHANNEL_VALUE_UPDATE_B 171
#define SUPLA_CS_CALL_GET_NEXT 180
#define SUPLA_SC_CALL_EVENT 190
#define SUPLA_CS_CALL_CHANNEL_SET_VALUE 200
#define SUPLA_CS_CALL_CHANNEL_SET_VALUE_B 205                 // ver. >= 3
#define SUPLA_DCS_CALL_SET_ACTIVITY_TIMEOUT 210               // ver. >= 2
#define SUPLA_SDC_CALL_SET_ACTIVITY_TIMEOUT_RESULT 220        // ver. >= 2
#define SUPLA_DS_CALL_GET_FIRMWARE_UPDATE_URL 300             // ver. >= 5
#define SUPLA_SD_CALL_GET_FIRMWARE_UPDATE_URL_RESULT 310      // ver. >= 5
#define SUPLA_DCS_CALL_GET_REGISTRATION_ENABLED 320           // ver. >= 7
#define SUPLA_SDC_CALL_GET_REGISTRATION_ENABLED_RESULT 330    // ver. >= 7
#define SUPLA_CS_CALL_OAUTH_TOKEN_REQUEST 340                 // ver. >= 10
#define SUPLA_SC_CALL_OAUTH_TOKEN_REQUEST_RESULT 350          // ver. >= 10
#define SUPLA_SC_CALL_CHANNELPACK_UPDATE_B 360                // ver. >= 8
#define SUPLA_SC_CALL_CHANNELPACK_UPDATE_C 361                // ver. >= 10
#define SUPLA_SC_CALL_CHANNELPACK_UPDATE_D 362                // ver. >= 15
#define SUPLA_SC_CALL_CHANNELPACK_UPDATE_E 363                // ver. >= 23
#define SUPLA_SC_CALL_CHANNELGROUP_PACK_UPDATE 380            // ver. >= 9
#define SUPLA_SC_CALL_CHANNELGROUP_PACK_UPDATE_B 381          // ver. >= 10
#define SUPLA_SC_CALL_CHANNELGROUP_RELATION_PACK_UPDATE 390   // ver. >= 9
#define SUPLA_SC_CALL_CHANNEL_RELATION_PACK_UPDATE 395        // ver. >= 21
#define SUPLA_SC_CALL_CHANNELVALUE_PACK_UPDATE 400            // ver. >= 9
#define SUPLA_SC_CALL_CHANNELVALUE_PACK_UPDATE_B 401          // ver. >= 15
#define SUPLA_SC_CALL_CHANNELEXTENDEDVALUE_PACK_UPDATE 405    // ver. >= 10
#define SUPLA_CS_CALL_SET_VALUE 410                           // ver. >= 9
#define SUPLA_CS_CALL_SUPERUSER_AUTHORIZATION_REQUEST 420     // ver. >= 10
#define SUPLA_CS_CALL_GET_SUPERUSER_AUTHORIZATION_RESULT 425  // ver. >= 12
#define SUPLA_SC_CALL_SUPERUSER_AUTHORIZATION_RESULT 430      // ver. >= 10
#define SUPLA_CS_CALL_DEVICE_CALCFG_REQUEST 440               // ver. >= 10
#define SUPLA_CS_CALL_DEVICE_CALCFG_REQUEST_B 445             // ver. >= 11
#define SUPLA_SC_CALL_DEVICE_CALCFG_RESULT 450                // ver. >= 10
#define SUPLA_SD_CALL_DEVICE_CALCFG_REQUEST 460               // ver. >= 10
#define SUPLA_DS_CALL_DEVICE_CALCFG_RESULT 470                // ver. >= 10
#define SUPLA_DCS_CALL_GET_USER_LOCALTIME 480                 // ver. >= 11
#define SUPLA_DCS_CALL_GET_USER_LOCALTIME_RESULT 490          // ver. >= 11
#define SUPLA_CSD_CALL_GET_CHANNEL_STATE 500                  // ver. >= 12
#define SUPLA_DSC_CALL_CHANNEL_STATE_RESULT 510               // ver. >= 12
#define SUPLA_CS_CALL_GET_CHANNEL_BASIC_CFG 520               // ver. >= 12
#define SUPLA_SC_CALL_CHANNEL_BASIC_CFG_RESULT 530            // ver. >= 12
#define SUPLA_CS_CALL_SET_CHANNEL_FUNCTION 540                // ver. >= 12
#define SUPLA_SC_CALL_SET_CHANNEL_FUNCTION_RESULT 550         // ver. >= 12
#define SUPLA_CS_CALL_CLIENTS_RECONNECT_REQUEST 560           // ver. >= 12
#define SUPLA_SC_CALL_CLIENTS_RECONNECT_REQUEST_RESULT 570    // ver. >= 12
#define SUPLA_CS_CALL_SET_REGISTRATION_ENABLED 580            // ver. >= 12
#define SUPLA_SC_CALL_SET_REGISTRATION_ENABLED_RESULT 590     // ver. >= 12
#define SUPLA_CS_CALL_DEVICE_RECONNECT_REQUEST 600            // ver. >= 12
#define SUPLA_SC_CALL_DEVICE_RECONNECT_REQUEST_RESULT 610     // ver. >= 12
#define SUPLA_DS_CALL_GET_CHANNEL_FUNCTIONS 620               // ver. >= 12
#define SUPLA_SD_CALL_GET_CHANNEL_FUNCTIONS_RESULT 630        // ver. >= 12
#define SUPLA_DCS_CALL_SET_CHANNEL_CAPTION 640                // ver. >= 12
#define SUPLA_SCD_CALL_SET_CHANNEL_CAPTION_RESULT 650         // ver. >= 12
#define SUPLA_CS_CALL_SET_CHANNEL_GROUP_CAPTION 642           // ver. >= 20
#define SUPLA_SC_CALL_SET_CHANNEL_GROUP_CAPTION_RESULT 652    // ver. >= 20
#define SUPLA_CS_CALL_SET_LOCATION_CAPTION 645                // ver. >= 14
#define SUPLA_SC_CALL_SET_LOCATION_CAPTION_RESULT 655         // ver. >= 14
#define SUPLA_DS_CALL_GET_CHANNEL_CONFIG 680                  // ver. >= 16
#define SUPLA_SD_CALL_GET_CHANNEL_CONFIG_RESULT 690           // ver. >= 16
#define SUPLA_DS_CALL_SET_CHANNEL_CONFIG 681                  // ver. >= 21
#define SUPLA_SD_CALL_SET_CHANNEL_CONFIG_RESULT 691           // ver. >= 21
#define SUPLA_SD_CALL_SET_CHANNEL_CONFIG 682                  // ver. >= 21
#define SUPLA_DS_CALL_SET_CHANNEL_CONFIG_RESULT 692           // ver. >= 21
#define SUPLA_SD_CALL_CHANNEL_CONFIG_FINISHED 683             // ver. >= 21
#define SUPLA_DS_CALL_SET_DEVICE_CONFIG 684                   // ver. >= 21
#define SUPLA_SD_CALL_SET_DEVICE_CONFIG_RESULT 694            // ver. >= 21
#define SUPLA_SD_CALL_SET_DEVICE_CONFIG 685                   // ver. >= 21
#define SUPLA_DS_CALL_SET_DEVICE_CONFIG_RESULT 695            // ver. >= 21
#define SUPLA_DS_CALL_ACTIONTRIGGER 700                       // ver. >= 16
#define SUPLA_CS_CALL_TIMER_ARM 800                           // ver. >= 17
#define SUPLA_SC_CALL_SCENE_PACK_UPDATE 900                   // ver. >= 18
#define SUPLA_SC_CALL_SCENE_STATE_PACK_UPDATE 910             // ver. >= 18
#define SUPLA_CS_CALL_EXECUTE_ACTION 1000                     // ver. >= 19
#define SUPLA_CS_CALL_EXECUTE_ACTION_WITH_AUTH 1010           // ver. >= 19
#define SUPLA_SC_CALL_ACTION_EXECUTION_RESULT 1020            // ver. >= 19
#define SUPLA_CS_CALL_GET_CHANNEL_VALUE_WITH_AUTH 1030        // ver. >= 19
#define SUPLA_SC_CALL_GET_CHANNEL_VALUE_RESULT 1040           // ver. >= 19
#define SUPLA_CS_CALL_SET_SCENE_CAPTION 1045                  // ver. >= 19
#define SUPLA_SC_CALL_SET_SCENE_CAPTION_RESULT 1055           // ver. >= 19
#define SUPLA_DS_CALL_REGISTER_PUSH_NOTIFICATION 1100         // ver. >= 20
#define SUPLA_DS_CALL_SEND_PUSH_NOTIFICATION 1110             // ver. >= 20
#define SUPLA_CS_CALL_REGISTER_PN_CLIENT_TOKEN 1120           // ver. >= 20
#define SUPLA_SC_CALL_REGISTER_PN_CLIENT_TOKEN_RESULT 1121    // ver. >= 20
#define SUPLA_CS_CALL_GET_CHANNEL_CONFIG 1200                 // ver. >= 21
#define SUPLA_SC_CALL_CHANNEL_CONFIG_UPDATE_OR_RESULT 1210    // ver. >= 21
#define SUPLA_CS_CALL_SET_CHANNEL_CONFIG 1220                 // ver. >= 21
#define SUPLA_CS_CALL_GET_DEVICE_CONFIG 1240                  // ver. >= 21
#define SUPLA_SC_CALL_DEVICE_CONFIG_UPDATE_OR_RESULT 1250     // ver. >= 21
#define SUPLA_DS_CALL_SET_SUBDEVICE_DETAILS 1260              // ver. >= 25

#define SUPLA_RESULT_RESPONSE_TIMEOUT -8
#define SUPLA_RESULT_CANT_CONNECT_TO_HOST -7
#define SUPLA_RESULT_HOST_NOT_FOUND -6
#define SUPLA_RESULT_CALL_NOT_ALLOWED -5
#define SUPLA_RESULT_DATA_TOO_LARGE -4
#define SUPLA_RESULT_BUFFER_OVERFLOW -3
#define SUPLA_RESULT_DATA_ERROR -2
#define SUPLA_RESULT_VERSION_ERROR -1
#define SUPLA_RESULT_FALSE 0
#define SUPLA_RESULT_TRUE 1

// SUPLA_RESULTCODE_ are sent in TSuplaDataPacket.data (unsigned char)
#define SUPLA_RESULTCODE_NONE 0
#define SUPLA_RESULTCODE_UNSUPORTED 1
#define SUPLA_RESULTCODE_FALSE 2
#define SUPLA_RESULTCODE_TRUE 3
#define SUPLA_RESULTCODE_TEMPORARILY_UNAVAILABLE 4
#define SUPLA_RESULTCODE_BAD_CREDENTIALS 5
#define SUPLA_RESULTCODE_LOCATION_CONFLICT 6
#define SUPLA_RESULTCODE_CHANNEL_CONFLICT 7
#define SUPLA_RESULTCODE_DEVICE_DISABLED 8
#define SUPLA_RESULTCODE_ACCESSID_DISABLED 9
#define SUPLA_RESULTCODE_LOCATION_DISABLED 10
#define SUPLA_RESULTCODE_CLIENT_DISABLED 11
#define SUPLA_RESULTCODE_CLIENT_LIMITEXCEEDED 12
#define SUPLA_RESULTCODE_DEVICE_LIMITEXCEEDED 13
#define SUPLA_RESULTCODE_GUID_ERROR 14
#define SUPLA_RESULTCODE_DEVICE_LOCKED 15                          // ver. >= 22
#define SUPLA_RESULTCODE_REGISTRATION_DISABLED 17                  // ver. >= 7
#define SUPLA_RESULTCODE_ACCESSID_NOT_ASSIGNED 18                  // ver. >= 7
#define SUPLA_RESULTCODE_AUTHKEY_ERROR 19                          // ver. >= 7
#define SUPLA_RESULTCODE_NO_LOCATION_AVAILABLE 20                  // ver. >= 7
#define SUPLA_RESULTCODE_USER_CONFLICT 21                          // Deprecated
#define SUPLA_RESULTCODE_UNAUTHORIZED 22                           // ver. >= 10
#define SUPLA_RESULTCODE_AUTHORIZED 23                             // ver. >= 10
#define SUPLA_RESULTCODE_NOT_ALLOWED 24                            // ver. >= 12
#define SUPLA_RESULTCODE_CHANNELNOTFOUND 25                        // ver. >= 12
#define SUPLA_RESULTCODE_UNKNOWN_ERROR 26                          // ver. >= 12
#define SUPLA_RESULTCODE_DENY_CHANNEL_BELONG_TO_GROUP 27           // ver. >= 12
#define SUPLA_RESULTCODE_DENY_CHANNEL_HAS_SCHEDULE 28              // ver. >= 12
#define SUPLA_RESULTCODE_DENY_CHANNEL_IS_ASSOCIETED_WITH_SCENE 29  // ver. >= 12
#define SUPLA_RESULTCODE_DENY_CHANNEL_IS_ASSOCIETED_WITH_ACTION_TRIGGER \
  30                                              // ver. >= 16
#define SUPLA_RESULTCODE_ACCESSID_INACTIVE 31     // ver. >= 17
#define SUPLA_RESULTCODE_CFG_MODE_REQUESTED 32    // ver. >= 18
#define SUPLA_RESULTCODE_ACTION_UNSUPPORTED 33    // ver. >= 19
#define SUPLA_RESULTCODE_SUBJECT_NOT_FOUND 34     // ver. >= 19
#define SUPLA_RESULTCODE_INCORRECT_PARAMETERS 35  // ver. >= 19
#define SUPLA_RESULTCODE_CLIENT_NOT_EXISTS 36     // ver. >= 19
#define SUPLA_RESULTCODE_COUNTRY_REJECTED 37
#define SUPLA_RESULTCODE_CHANNEL_IS_OFFLINE 38                    // ver. >= 19
#define SUPLA_RESULTCODE_NOT_REGISTERED 39                        // ver. >= 20
#define SUPLA_RESULTCODE_DENY_CHANNEL_IS_ASSOCIETED_WITH_VBT 40   // >= 20
#define SUPLA_RESULTCODE_DENY_CHANNEL_IS_ASSOCIETED_WITH_PUSH 41  // >= 20
#define SUPLA_RESULTCODE_RESTART_REQUESTED 42                     // ver. >= 25
#define SUPLA_RESULTCODE_IDENTIFY_REQUESTED 43                    // ver. >= 25
#define SUPLA_RESULTCODE_MALFORMED_EMAIL 44                       // ver. >= ?

#define SUPLA_OAUTH_RESULTCODE_ERROR 0         // ver. >= 10
#define SUPLA_OAUTH_RESULTCODE_SUCCESS 1       // ver. >= 10
#define SUPLA_OAUTH_TEMPORARILY_UNAVAILABLE 2  // ver. >= 10

#define SUPLA_DEVICE_NAME_MAXSIZE 201
#define SUPLA_CLIENT_NAME_MAXSIZE 201
#define SUPLA_SENDER_NAME_MAXSIZE 201
#define SUPLA_INITIATOR_NAME_MAXSIZE SUPLA_SENDER_NAME_MAXSIZE

#define SUPLA_CHANNELMAXCOUNT 128

#define SUPLA_CHANNELVALUE_SIZE 8

#if defined(__AVR__) || defined(ESP8266) || defined(ESP32) || \
    defined(ESP_PLATFORM) || defined(ARDUINO) || defined(SUPLA_DEVICE)
#define SUPLA_CHANNELEXTENDEDVALUE_SIZE 256
#else
#define SUPLA_CHANNELEXTENDEDVALUE_SIZE 1024
#endif

#define SUPLA_CHANNELTYPE_SENSORNO 1000  // name DEPRECATED
// use BINARYSENSOR instead
#define SUPLA_CHANNELTYPE_BINARYSENSOR 1000
#define SUPLA_CHANNELTYPE_SENSORNC 1010        // DEPRECATED
#define SUPLA_CHANNELTYPE_DISTANCESENSOR 1020  // ver. >= 5
#define SUPLA_CHANNELTYPE_CALLBUTTON 1500      // ver. >= 4
#define SUPLA_CHANNELTYPE_RELAYHFD4 2000       // DEPRECATED
#define SUPLA_CHANNELTYPE_RELAYG5LA1A 2010     // DEPRECATED
#define SUPLA_CHANNELTYPE_2XRELAYG5LA1A 2020   // DEPRECATED
#define SUPLA_CHANNELTYPE_RELAY 2900
#define SUPLA_CHANNELTYPE_THERMOMETERDS18B20 3000  // DEPRECATED
#define SUPLA_CHANNELTYPE_DHT11 3010               // ver. >= 4  DEPRECATED
#define SUPLA_CHANNELTYPE_DHT22 3020               // ver. >= 4  DEPRECATED
#define SUPLA_CHANNELTYPE_DHT21 3022               // ver. >= 5  DEPRECATED
#define SUPLA_CHANNELTYPE_AM2302 3030              // ver. >= 4  DEPRECATED
#define SUPLA_CHANNELTYPE_AM2301 3032              // ver. >= 5  DEPRECATED

#define SUPLA_CHANNELTYPE_THERMOMETER 3034            // ver. >= 8
#define SUPLA_CHANNELTYPE_HUMIDITYSENSOR 3036         // ver. >= 8
#define SUPLA_CHANNELTYPE_HUMIDITYANDTEMPSENSOR 3038  // ver. >= 8
#define SUPLA_CHANNELTYPE_WINDSENSOR 3042             // ver. >= 8
#define SUPLA_CHANNELTYPE_PRESSURESENSOR 3044         // ver. >= 8
#define SUPLA_CHANNELTYPE_RAINSENSOR 3048             // ver. >= 8
#define SUPLA_CHANNELTYPE_WEIGHTSENSOR 3050           // ver. >= 8
#define SUPLA_CHANNELTYPE_WEATHER_STATION 3100        // ver. >= 8

#define SUPLA_CHANNELTYPE_DIMMER 4000            // ver. >= 4
#define SUPLA_CHANNELTYPE_RGBLEDCONTROLLER 4010  // ver. >= 4
#define SUPLA_CHANNELTYPE_DIMMERANDRGBLED 4020   // ver. >= 4

#define SUPLA_CHANNELTYPE_ELECTRICITY_METER 5000  // ver. >= 10
#define SUPLA_CHANNELTYPE_IMPULSE_COUNTER 5010    // ver. >= 10

#define SUPLA_CHANNELTYPE_THERMOSTAT 6000                   // ver. >= 11
#define SUPLA_CHANNELTYPE_THERMOSTAT_HEATPOL_HOMEPLUS 6010  // ver. >= 11
#define SUPLA_CHANNELTYPE_HVAC 6100                         // ver. >= 21

#define SUPLA_CHANNELTYPE_VALVE_OPENCLOSE 7000              // ver. >= 12
#define SUPLA_CHANNELTYPE_VALVE_PERCENTAGE 7010             // ver. >= 12
#define SUPLA_CHANNELTYPE_BRIDGE 8000                       // ver. >= 12
#define SUPLA_CHANNELTYPE_GENERAL_PURPOSE_MEASUREMENT 9000  // ver. >= 23
#define SUPLA_CHANNELTYPE_GENERAL_PURPOSE_METER 9010        // ver. >= 23
#define SUPLA_CHANNELTYPE_ENGINE 10000                      // ver. >= 12
#define SUPLA_CHANNELTYPE_ACTIONTRIGGER 11000               // ver. >= 16
#define SUPLA_CHANNELTYPE_DIGIGLASS 12000                   // ver. >= 12

#define SUPLA_CHANNELDRIVER_MCP23008 2

#define SUPLA_CHANNELFNC_NONE 0
#define SUPLA_CHANNELFNC_CONTROLLINGTHEGATEWAYLOCK 10
#define SUPLA_CHANNELFNC_CONTROLLINGTHEGATE 20
#define SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR 30
#define SUPLA_CHANNELFNC_THERMOMETER 40
#define SUPLA_CHANNELFNC_HUMIDITY 42
#define SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE 45
#define SUPLA_CHANNELFNC_OPENINGSENSOR_GATEWAY 50
#define SUPLA_CHANNELFNC_OPENINGSENSOR_GATE 60
#define SUPLA_CHANNELFNC_OPENINGSENSOR_GARAGEDOOR 70
#define SUPLA_CHANNELFNC_NOLIQUIDSENSOR 80
#define SUPLA_CHANNELFNC_CONTROLLINGTHEDOORLOCK 90
#define SUPLA_CHANNELFNC_OPENINGSENSOR_DOOR 100
#define SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER 110
#define SUPLA_CHANNELFNC_CONTROLLINGTHEROOFWINDOW 115  // ver. >= 13
#define SUPLA_CHANNELFNC_OPENINGSENSOR_ROLLERSHUTTER 120
#define SUPLA_CHANNELFNC_OPENINGSENSOR_ROOFWINDOW 125  // ver. >= 13
#define SUPLA_CHANNELFNC_POWERSWITCH 130
#define SUPLA_CHANNELFNC_LIGHTSWITCH 140
#define SUPLA_CHANNELFNC_RING 150
#define SUPLA_CHANNELFNC_ALARM 160
#define SUPLA_CHANNELFNC_NOTIFICATION 170
#define SUPLA_CHANNELFNC_DIMMER 180
#define SUPLA_CHANNELFNC_RGBLIGHTING 190
#define SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING 200
#define SUPLA_CHANNELFNC_DEPTHSENSOR 210                   // ver. >= 5
#define SUPLA_CHANNELFNC_DISTANCESENSOR 220                // ver. >= 5
#define SUPLA_CHANNELFNC_OPENINGSENSOR_WINDOW 230          // ver. >= 8
#define SUPLA_CHANNELFNC_HOTELCARDSENSOR 235               // ver. >= 21
#define SUPLA_CHANNELFNC_ALARMARMAMENTSENSOR 236           // ver. >= 21
#define SUPLA_CHANNELFNC_MAILSENSOR 240                    // ver. >= 8
#define SUPLA_CHANNELFNC_WINDSENSOR 250                    // ver. >= 8
#define SUPLA_CHANNELFNC_PRESSURESENSOR 260                // ver. >= 8
#define SUPLA_CHANNELFNC_RAINSENSOR 270                    // ver. >= 8
#define SUPLA_CHANNELFNC_WEIGHTSENSOR 280                  // ver. >= 8
#define SUPLA_CHANNELFNC_WEATHER_STATION 290               // ver. >= 8
#define SUPLA_CHANNELFNC_STAIRCASETIMER 300                // ver. >= 8
#define SUPLA_CHANNELFNC_ELECTRICITY_METER 310             // ver. >= 10
#define SUPLA_CHANNELFNC_IC_ELECTRICITY_METER 315          // ver. >= 12
#define SUPLA_CHANNELFNC_IC_GAS_METER 320                  // ver. >= 10
#define SUPLA_CHANNELFNC_IC_WATER_METER 330                // ver. >= 10
#define SUPLA_CHANNELFNC_IC_HEAT_METER 340                 // ver. >= 10
#define SUPLA_CHANNELFNC_IC_EVENTS 350                     // ver. >= 21
#define SUPLA_CHANNELFNC_IC_SECONDS 360                    // ver. >= 21
#define SUPLA_CHANNELFNC_THERMOSTAT_HEATPOL_HOMEPLUS 410   // ver. >= 11
#define SUPLA_CHANNELFNC_HVAC_THERMOSTAT 420               // ver. >= 21
#define SUPLA_CHANNELFNC_HVAC_THERMOSTAT_HEAT_COOL 422     // ver. >= 21
#define SUPLA_CHANNELFNC_HVAC_DRYER 423                    // ver. >= 21
#define SUPLA_CHANNELFNC_HVAC_FAN 424                      // ver. >= 21
#define SUPLA_CHANNELFNC_HVAC_THERMOSTAT_DIFFERENTIAL 425  // ver. >= 21
#define SUPLA_CHANNELFNC_HVAC_DOMESTIC_HOT_WATER 426       // ver. >= 21
#define SUPLA_CHANNELFNC_VALVE_OPENCLOSE 500               // ver. >= 12
#define SUPLA_CHANNELFNC_VALVE_PERCENTAGE 510              // ver. >= 12
#define SUPLA_CHANNELFNC_GENERAL_PURPOSE_MEASUREMENT 520   // ver. >= 23
#define SUPLA_CHANNELFNC_GENERAL_PURPOSE_METER 530         // ver. >= 23
#define SUPLA_CHANNELFNC_CONTROLLINGTHEENGINESPEED 600     // ver. >= 12
#define SUPLA_CHANNELFNC_ACTIONTRIGGER 700                 // ver. >= 16
#define SUPLA_CHANNELFNC_DIGIGLASS_HORIZONTAL 800          // ver. >= 14
#define SUPLA_CHANNELFNC_DIGIGLASS_VERTICAL 810            // ver. >= 14
#define SUPLA_CHANNELFNC_CONTROLLINGTHEFACADEBLIND 900     // ver. >= 24
#define SUPLA_CHANNELFNC_TERRACE_AWNING 910                // ver. >= 24
#define SUPLA_CHANNELFNC_PROJECTOR_SCREEN 920              // ver. >= 24
#define SUPLA_CHANNELFNC_CURTAIN 930                       // ver. >= 24
#define SUPLA_CHANNELFNC_VERTICAL_BLIND 940                // ver. >= 24
#define SUPLA_CHANNELFNC_ROLLER_GARAGE_DOOR 950            // ver. >= 24
#define SUPLA_CHANNELFNC_PUMPSWITCH 960                    // ver. >= 25
#define SUPLA_CHANNELFNC_HEATORCOLDSOURCESWITCH 970        // ver. >= 25

#define SUPLA_BIT_FUNC_CONTROLLINGTHEGATEWAYLOCK 0x00000001
#define SUPLA_BIT_FUNC_CONTROLLINGTHEGATE 0x00000002
#define SUPLA_BIT_FUNC_CONTROLLINGTHEGARAGEDOOR 0x00000004
#define SUPLA_BIT_FUNC_CONTROLLINGTHEDOORLOCK 0x00000008
#define SUPLA_BIT_FUNC_CONTROLLINGTHEROLLERSHUTTER 0x00000010
#define SUPLA_BIT_FUNC_POWERSWITCH 0x00000020
#define SUPLA_BIT_FUNC_LIGHTSWITCH 0x00000040
#define SUPLA_BIT_FUNC_STAIRCASETIMER 0x00000080                // ver. >= 8
#define SUPLA_BIT_FUNC_THERMOMETER 0x00000100                   // ver. >= 12
#define SUPLA_BIT_FUNC_HUMIDITYANDTEMPERATURE 0x00000200        // ver. >= 12
#define SUPLA_BIT_FUNC_HUMIDITY 0x00000400                      // ver. >= 12
#define SUPLA_BIT_FUNC_WINDSENSOR 0x00000800                    // ver. >= 12
#define SUPLA_BIT_FUNC_PRESSURESENSOR 0x00001000                // ver. >= 12
#define SUPLA_BIT_FUNC_RAINSENSOR 0x00002000                    // ver. >= 12
#define SUPLA_BIT_FUNC_WEIGHTSENSOR 0x00004000                  // ver. >= 12
#define SUPLA_BIT_FUNC_CONTROLLINGTHEROOFWINDOW 0x00008000      // ver. >= 13
#define SUPLA_BIT_FUNC_CONTROLLINGTHEFACADEBLIND 0x00010000     // ver. >= 17
#define SUPLA_BIT_FUNC_HVAC_THERMOSTAT 0x00020000               // ver. >= 21
#define SUPLA_BIT_FUNC_HVAC_THERMOSTAT_HEAT_COOL 0x00040000     // ver. >= 21
#define SUPLA_BIT_FUNC_HVAC_THERMOSTAT_DIFFERENTIAL 0x00080000  // ver. >= 21
#define SUPLA_BIT_FUNC_HVAC_DOMESTIC_HOT_WATER 0x00100000       // ver. >= 21
#define SUPLA_BIT_FUNC_TERRACE_AWNING 0x00200000                // ver. >= 24
#define SUPLA_BIT_FUNC_PROJECTOR_SCREEN 0x00400000              // ver. >= 24
#define SUPLA_BIT_FUNC_CURTAIN 0x00800000                       // ver. >= 24
#define SUPLA_BIT_FUNC_VERTICAL_BLIND 0x01000000                // ver. >= 24
#define SUPLA_BIT_FUNC_ROLLER_GARAGE_DOOR 0x02000000            // ver. >= 24
#define SUPLA_BIT_FUNC_PUMPSWITCH 0x04000000                    // ver. >= 25
#define SUPLA_BIT_FUNC_HEATORCOLDSOURCESWITCH 0x08000000        // ver. >= 25

#define SUPLA_EVENT_CONTROLLINGTHEGATEWAYLOCK 10
#define SUPLA_EVENT_CONTROLLINGTHEGATE 20
#define SUPLA_EVENT_CONTROLLINGTHEGARAGEDOOR 30
#define SUPLA_EVENT_CONTROLLINGTHEDOORLOCK 40
#define SUPLA_EVENT_CONTROLLINGTHEROLLERSHUTTER 50
#define SUPLA_EVENT_TERRACE_AWNING 51      // ver. >= 24
#define SUPLA_EVENT_CURTAIN 52             // ver. >= 24
#define SUPLA_EVENT_PROJECTOR_SCREEN 53    // ver. >= 24
#define SUPLA_EVENT_ROLLER_GARAGE_DOOR 54  // ver. >= 24
#define SUPLA_EVENT_CONTROLLINGTHEROOFWINDOW 55
#define SUPLA_EVENT_CONTROLLINGTHEFACADEBLIND 56  // ver. >= 24
#define SUPLA_EVENT_VERTICAL_BLIND 57             // ver. >= 24
#define SUPLA_EVENT_POWERONOFF 60
#define SUPLA_EVENT_LIGHTONOFF 70
#define SUPLA_EVENT_STAIRCASETIMERONOFF 80       // ver. >= 9
#define SUPLA_EVENT_VALVEOPENCLOSE 90            // ver. >= 12
#define SUPLA_EVENT_SET_BRIDGE_VALUE_FAILED 100  // ver. >= 12

#define SUPLA_URL_PROTO_HTTP 0x01
#define SUPLA_URL_PROTO_HTTPS 0x02

#define SUPLA_PLATFORM_UNKNOWN 0
#define SUPLA_PLATFORM_ESP8266 1

#define SUPLA_TARGET_CHANNEL 0
#define SUPLA_TARGET_GROUP 1
#define SUPLA_TARGET_IODEVICE 2

#define SUPLA_MFR_UNKNOWN 0
#define SUPLA_MFR_ACSOFTWARE 1
#define SUPLA_MFR_TRANSCOM 2
#define SUPLA_MFR_LOGI 3
#define SUPLA_MFR_ZAMEL 4
#define SUPLA_MFR_NICE 5
#define SUPLA_MFR_ITEAD 6
#define SUPLA_MFR_DOYLETRATT 7
#define SUPLA_MFR_HEATPOL 8
#define SUPLA_MFR_FAKRO 9
#define SUPLA_MFR_PEVEKO 10
#define SUPLA_MFR_WEKTA 11
#define SUPLA_MFR_STA_SYSTEM 12
#define SUPLA_MFR_DGF 13
#define SUPLA_MFR_COMELIT 14
#define SUPLA_MFR_POLIER 15
#define SUPLA_MFR_ERGO_ENERGIA 16
#define SUPLA_MFR_SOMEF 17
#define SUPLA_MFR_AURATON 18
#define SUPLA_MFR_HPD 19

// BIT map definition for TDS_SuplaRegisterDevice_*::Flags (32 bit)
#define SUPLA_DEVICE_FLAG_CALCFG_ENTER_CFG_MODE 0x0010          // ver. >= 17
#define SUPLA_DEVICE_FLAG_SLEEP_MODE_ENABLED 0x0020             // ver. >= 18
#define SUPLA_DEVICE_FLAG_CALCFG_SET_TIME 0x0040                // ver. >= 21
#define SUPLA_DEVICE_FLAG_DEVICE_CONFIG_SUPPORTED 0x0080        // ver. >= 21
#define SUPLA_DEVICE_FLAG_DEVICE_LOCKED 0x0100                  // ver. >= 22
#define SUPLA_DEVICE_FLAG_CALCFG_SUBDEVICE_PAIRING 0x0200       // ver. >= 25
#define SUPLA_DEVICE_FLAG_CALCFG_IDENTIFY_DEVICE 0x0400         // ver. >= 25
#define SUPLA_DEVICE_FLAG_CALCFG_RESTART_DEVICE 0x0800          // ver. >= 25
#define SUPLA_DEVICE_FLAG_ALWAYS_ALLOW_CHANNEL_DELETION 0x1000  // ver. >= 25
#define SUPLA_DEVICE_FLAG_BLOCK_ADDING_CHANNELS_AFTER_DELETION \
  0x2000  // ver. >= 25

// BIT map definition for TDS_SuplaRegisterDevice_F::ConfigFields (64 bit)
// type: TDeviceConfig_StatusLed
#define SUPLA_DEVICE_CONFIG_FIELD_STATUS_LED (1ULL << 0)  // v. >= 21
// type: TDeviceConfig_ScreenBrightness
#define SUPLA_DEVICE_CONFIG_FIELD_SCREEN_BRIGHTNESS (1ULL << 1)  // v. >= 21
// type: TDeviceConfig_ButtonVolume
#define SUPLA_DEVICE_CONFIG_FIELD_BUTTON_VOLUME (1ULL << 2)  // v. >= 21
// type: TDeviceConfig_DisableUserInterface
#define SUPLA_DEVICE_CONFIG_FIELD_DISABLE_USER_INTERFACE \
  (1ULL << 3)  // v. >= 21
// type: TDeviceConfig_AutomaticTimeSync
#define SUPLA_DEVICE_CONFIG_FIELD_AUTOMATIC_TIME_SYNC (1ULL << 4)  // v. >= 21
// type: TDeviceConfig_HomeScreenOffDelay
#define SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_OFF_DELAY (1ULL << 5)  // v. >= 21
// type: TDeviceConfig_HomeScreenContent
#define SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_CONTENT (1ULL << 6)  // v. >= 21
// type: TDeviceConfig_HomeScreenOffDelayType
#define SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_OFF_DELAY_TYPE \
  (1ULL << 7)  // v. >= 24
// type: TDeviceConfig_PowerStatusLed
#define SUPLA_DEVICE_CONFIG_FIELD_POWER_STATUS_LED (1ULL << 8)  // v. >= 25

// BIT map definition for TDS_SuplaDeviceChannel_C::Flags (32 bit)
#define SUPLA_CHANNEL_FLAG_ZWAVE_BRIDGE 0x0001  // ver. >= 12
#define SUPLA_CHANNEL_FLAG_IR_BRIDGE 0x0002     // ver. >= 12
#define SUPLA_CHANNEL_FLAG_RF_BRIDGE 0x0004     // ver. >= 12
// Free bit for future use: 0x0008
#define SUPLA_CHANNEL_FLAG_CHART_TYPE_BAR \
  0x0010  // ver. >= 12
          // DEPRECATED
#define SUPLA_CHANNEL_FLAG_CHART_DS_TYPE_DIFFERENTAL \
  0x0020  // ver. >= 12
          // DEPRECATED
#define SUPLA_CHANNEL_FLAG_CHART_INTERPOLATE_MEASUREMENTS \
  0x0040                                                   // ver. >= 12
                                                           // DEPRECATED
#define SUPLA_CHANNEL_FLAG_RS_SBS_AND_STOP_ACTIONS 0x0080  // ver. >= 17
#define SUPLA_CHANNEL_FLAG_RGBW_COMMANDS_SUPPORTED 0x0100  // ver. >= 21
// Free bits for future use:  0x0200, 0x0400, 0x0800
#define SUPLA_CHANNEL_FLAG_RS_AUTO_CALIBRATION 0x1000              // ver. >= 15
#define SUPLA_CHANNEL_FLAG_CALCFG_RESET_COUNTERS 0x2000            // ver. >= 15
#define SUPLA_CHANNEL_FLAG_CALCFG_RECALIBRATE 0x4000               // ver. >= 15
#define SUPLA_CHANNEL_FLAG_CALCFG_IDENTIFY_SUBDEVICE 0x8000        // ver. >= 25
#define SUPLA_CHANNEL_FLAG_CHANNELSTATE 0x00010000                 // ver. >= 12
#define SUPLA_CHANNEL_FLAG_PHASE1_UNSUPPORTED 0x00020000           // ver. >= 12
#define SUPLA_CHANNEL_FLAG_PHASE2_UNSUPPORTED 0x00040000           // ver. >= 12
#define SUPLA_CHANNEL_FLAG_PHASE3_UNSUPPORTED 0x00080000           // ver. >= 12
#define SUPLA_CHANNEL_FLAG_TIME_SETTING_NOT_AVAILABLE 0x00100000   // ver. >= 12
#define SUPLA_CHANNEL_FLAG_RSA_ENCRYPTED_PIN_REQUIRED 0x00200000   // ver. >= 12
#define SUPLA_CHANNEL_FLAG_OFFLINE_DURING_REGISTRATION 0x00400000  // ver. >= 12
#define SUPLA_CHANNEL_FLAG_ZIGBEE_BRIDGE 0x00800000                // ver. >= 12
#define SUPLA_CHANNEL_FLAG_COUNTDOWN_TIMER_SUPPORTED 0x01000000    // ver. >= 12
#define SUPLA_CHANNEL_FLAG_LIGHTSOURCELIFESPAN_SETTABLE \
  0x02000000  // ver. >= 12
#define SUPLA_CHANNEL_FLAG_POSSIBLE_SLEEP_MODE_deprecated \
  0x04000000  // ver. >= 12  DEPRECATED
#define SUPLA_CHANNEL_FLAG_RUNTIME_CHANNEL_CONFIG_UPDATE \
  0x08000000                                                    // ver. >= 21
#define SUPLA_CHANNEL_FLAG_WEEKLY_SCHEDULE 0x10000000           // ver. >= 21
#define SUPLA_CHANNEL_FLAG_HAS_PARENT 0x20000000                // ver. >= 21
#define SUPLA_CHANNEL_FLAG_CALCFG_RESTART_SUBDEVICE 0x40000000  // ver. >= 25
#define SUPLA_CHANNEL_FLAG_BATTERY_COVER_AVAILABLE  0x80000000  // ver. >= 25
#pragma pack(push, 1)

typedef struct {
  char tag[SUPLA_TAG_SIZE];
  unsigned char version;
  unsigned _supla_int_t rr_id;  // Request/Response ID
  unsigned _supla_int_t call_id;
  unsigned _supla_int_t data_size;
  char data[SUPLA_MAX_DATA_SIZE];  // Last variable in struct!
} TSuplaDataPacket;

typedef struct {
  // server -> device|client
  unsigned char proto_version_min;
  unsigned char proto_version;
  char SoftVer[SUPLA_SOFTVER_MAXSIZE];
} TSDC_SuplaGetVersionResult;

typedef struct {
  // server -> device|client
  unsigned char server_version_min;
  unsigned char server_version;
} TSDC_SuplaVersionError;

typedef struct {
  // device|client -> server
  struct _supla_timeval now;
} TDCS_SuplaPingServer;

// Compatibility with ESP8266
struct timeval_compat {
  _supla_int_t tv_sec;
  _supla_int_t tv_usec;
};

typedef struct {
  // device|client -> server
  struct timeval_compat now;
} TDCS_SuplaPingServer_COMPAT;

typedef struct {
  // server -> device|client
  struct _supla_timeval now;
} TSDC_SuplaPingServerResult;

typedef struct {
  // device|client -> server
  unsigned char activity_timeout;
} TDCS_SuplaSetActivityTimeout;

typedef struct {
  // server -> device|client
  unsigned char activity_timeout;
  unsigned char min;
  unsigned char max;
} TSDC_SuplaSetActivityTimeoutResult;

typedef struct {
  char value[SUPLA_CHANNELVALUE_SIZE];
  char sub_value[SUPLA_CHANNELVALUE_SIZE];  // For example sensor value
} TSuplaChannelValue;

typedef struct {
  char value[SUPLA_CHANNELVALUE_SIZE];
  char sub_value[SUPLA_CHANNELVALUE_SIZE];  // For example sensor value
  char sub_value_type;                      // SUBV_TYPE_
} TSuplaChannelValue_B;

#define SUBV_TYPE_NOT_SET_OR_OFFLINE 0
#define SUBV_TYPE_SENSOR 1
#define SUBV_TYPE_ELECTRICITY_MEASUREMENTS 2
#define SUBV_TYPE_IC_MEASUREMENTS 3

#define EV_TYPE_NONE 0
#ifdef USE_DEPRECATED_EMEV_V1
#define EV_TYPE_ELECTRICITY_METER_MEASUREMENT_V1 10
#endif /*USE_DEPRECATED_EMEV_V1*/
#ifdef USE_DEPRECATED_EMEV_V2
#define EV_TYPE_ELECTRICITY_METER_MEASUREMENT_V2 12
#endif /*USE_DEPRECATED_EMEV_V2*/
#define EV_TYPE_ELECTRICITY_METER_MEASUREMENT_V3 14
#define EV_TYPE_IMPULSE_COUNTER_DETAILS_V1 20
#define EV_TYPE_THERMOSTAT_DETAILS_V1 30
#define EV_TYPE_CHANNEL_STATE_V1 40
#define EV_TYPE_TIMER_STATE_V1 50
#define EV_TYPE_TIMER_STATE_V1_SEC 51
#define EV_TYPE_CHANNEL_AND_TIMER_STATE_V1 60
#define EV_TYPE_MULTI_VALUE 100

#define CALCFG_TYPE_THERMOSTAT_DETAILS_V1 10

typedef struct {
  char type;  // EV_TYPE_
  unsigned _supla_int_t size;
  char value[SUPLA_CHANNELEXTENDEDVALUE_SIZE];  // Last variable in struct!
} TSuplaChannelExtendedValue;                   // v. >= 10

typedef struct {
  // server -> client
  char EOL;  // End Of List
  _supla_int_t Id;
  unsigned _supla_int_t
      CaptionSize;  // including the terminating null byte ('\0')
  char Caption[SUPLA_LOCATION_CAPTION_MAXSIZE];  // Last variable in struct!
} TSC_SuplaLocation;

typedef struct {
  // server -> client
  _supla_int_t count;
  _supla_int_t total_left;
  TSC_SuplaLocation
      items[SUPLA_LOCATIONPACK_MAXCOUNT];  // Last variable in struct!
} TSC_SuplaLocationPack;

typedef struct {
  // device -> server
  unsigned char Number;
  _supla_int_t Type;
  char value[SUPLA_CHANNELVALUE_SIZE];
} TDS_SuplaDeviceChannel;

typedef struct {
  // device -> server

  _supla_int_t LocationID;
  char LocationPWD[SUPLA_LOCATION_PWD_MAXSIZE];  // UTF8

  char GUID[SUPLA_GUID_SIZE];
  char Name[SUPLA_DEVICE_NAME_MAXSIZE];  // UTF8
  char SoftVer[SUPLA_SOFTVER_MAXSIZE];

  unsigned char channel_count;
  TDS_SuplaDeviceChannel
      channels[SUPLA_CHANNELMAXCOUNT];  // Last variable in struct!
} TDS_SuplaRegisterDevice;

typedef struct {
  // device -> server

  unsigned char Number;
  _supla_int_t Type;

  _supla_int_t FuncList;
  _supla_int_t Default;

  char value[SUPLA_CHANNELVALUE_SIZE];
} TDS_SuplaDeviceChannel_B;  // ver. >= 2

typedef struct {
  unsigned char relatedChannelNumber;  // ChannelNumber + 1.
  unsigned _supla_int_t disablesLocalOperation;
} TActionTriggerProperties;

#define SUPLA_HVAC_VALUE_FLAG_SETPOINT_TEMP_HEAT_SET (1ULL << 0)
#define SUPLA_HVAC_VALUE_FLAG_SETPOINT_TEMP_COOL_SET (1ULL << 1)
// Tells if output responsible for heating function is enabled
#define SUPLA_HVAC_VALUE_FLAG_HEATING (1ULL << 2)
// Tells if output responsible for cooling function is enabled
#define SUPLA_HVAC_VALUE_FLAG_COOLING (1ULL << 3)
// Tells if channel works in Weekly Schedule mode. If set to 0, then it works
// in manual mode
#define SUPLA_HVAC_VALUE_FLAG_WEEKLY_SCHEDULE (1ULL << 4)
#define SUPLA_HVAC_VALUE_FLAG_COUNTDOWN_TIMER (1ULL << 5)
// FAN flag tells if fan function is enabled
#define SUPLA_HVAC_VALUE_FLAG_FAN_ENABLED (1ULL << 6)
#define SUPLA_HVAC_VALUE_FLAG_THERMOMETER_ERROR (1ULL << 7)
#define SUPLA_HVAC_VALUE_FLAG_CLOCK_ERROR (1ULL << 8)
#define SUPLA_HVAC_VALUE_FLAG_FORCED_OFF_BY_SENSOR (1ULL << 9)
// Only for SUPLA_CHANNELFNC_HVAC_THERMOSTAT
// If the COOL flag is not set, it means that the thermostat is working in
// heating mode.
#define SUPLA_HVAC_VALUE_FLAG_COOL (1ULL << 10)
#define SUPLA_HVAC_VALUE_FLAG_WEEKLY_SCHEDULE_TEMPORAL_OVERRIDE (1ULL << 11)
#define SUPLA_HVAC_VALUE_FLAG_BATTERY_COVER_OPEN (1ULL << 12)

// HVAC modes are used in channel value (as a command from server or
// as a status response from device to server) and in weekly schedules
// programs. Programs can't use value TURN_ON and WEEKLY_SCHEDULE
// Use SUPLA_HVAC_MODE_NOT_SET if you don't want to modify current mode, but
// only to alter tempreature setpoints.
#define SUPLA_HVAC_MODE_NOT_SET 0
#define SUPLA_HVAC_MODE_OFF 1
#define SUPLA_HVAC_MODE_HEAT 2
#define SUPLA_HVAC_MODE_COOL 3
#define SUPLA_HVAC_MODE_HEAT_COOL 4
#define SUPLA_HVAC_MODE_FAN_ONLY 6
#define SUPLA_HVAC_MODE_DRY 7
// TURN ON mode is a command. Device will use it to turn on and then it will
// restore previous mode which was used before turn off.
#define SUPLA_HVAC_MODE_CMD_TURN_ON 8
// Weekly schedule is a command. Device will use it to enable weekly schedule
// mode and then it will set its mode according to schedule
#define SUPLA_HVAC_MODE_CMD_WEEKLY_SCHEDULE 9
// Switch to manual mode - it will restore previously used manual mode and
// restore manual mode temperature setpoints when no new setpoints are given
// in value.
// It can be also used to switch to default manual mode, when no manual mode
// was used earlier.
#define SUPLA_HVAC_MODE_CMD_SWITCH_TO_MANUAL 10

typedef struct {
  unsigned char IsOn;  // DS: 0/1 (for on/off) or 2..102 (for 0-100%)
  unsigned char Mode;  // SUPLA_HVAC_MODE_
  _supla_int16_t
      SetpointTemperatureHeat;  // * 0.01 Celcius degree - used for heating
  _supla_int16_t
      SetpointTemperatureCool;    // * 0.01 - Celcius degree used for cooling
  unsigned _supla_int16_t Flags;  // SUPLA_HVAC_VALUE_FLAG_
} THVACValue;

typedef struct {
  // device -> server

  unsigned char Number;
  _supla_int_t Type;

  union {
    _supla_int_t FuncList;
    unsigned _supla_int_t ActionTriggerCaps;  // ver. >= 16
  };

  _supla_int_t Default;
  _supla_int_t Flags;

  union {
    char value[SUPLA_CHANNELVALUE_SIZE];
    TActionTriggerProperties actionTriggerProperties;  // ver. >= 16
    THVACValue hvacValue;
  };
} TDS_SuplaDeviceChannel_C;  // ver. >= 10

typedef struct {
  // device -> server

  unsigned char Number;
  _supla_int_t Type;

  union {
    _supla_int_t FuncList;
    unsigned _supla_int_t ActionTriggerCaps;  // ver. >= 16
  };

  _supla_int_t Default;
  _supla_int64_t Flags;

  unsigned char
      Offline;  // If true, the ValidityTimeSec and value variables are ignored.
  unsigned _supla_int_t ValueValidityTimeSec;

  union {
    char value[SUPLA_CHANNELVALUE_SIZE];
    TActionTriggerProperties actionTriggerProperties;  // ver. >= 16
    THVACValue hvacValue;
  };

  unsigned char DefaultIcon;
} TDS_SuplaDeviceChannel_D;  // ver. >= 23

typedef struct {
  // device -> server

  unsigned char Number;
  _supla_int_t Type;

  union {
    _supla_int_t FuncList;
    unsigned _supla_int_t ActionTriggerCaps;  // ver. >= 16
  };

  _supla_int_t Default;
  _supla_int64_t Flags;

  unsigned char
      Offline;  // If true, the ValidityTimeSec and value variables are ignored.
  unsigned _supla_int_t ValueValidityTimeSec;

  union {
    char value[SUPLA_CHANNELVALUE_SIZE];
    TActionTriggerProperties actionTriggerProperties;  // ver. >= 16
    THVACValue hvacValue;
  };

  unsigned char DefaultIcon;
  unsigned char SubDeviceId;  // 0 - no subdevice, 1..255 - subdevice id
} TDS_SuplaDeviceChannel_E;   // ver. >= 25

typedef struct {
  // device -> server

  _supla_int_t LocationID;
  char LocationPWD[SUPLA_LOCATION_PWD_MAXSIZE];  // UTF8

  char GUID[SUPLA_GUID_SIZE];
  char Name[SUPLA_DEVICE_NAME_MAXSIZE];  // UTF8
  char SoftVer[SUPLA_SOFTVER_MAXSIZE];

  unsigned char channel_count;
  TDS_SuplaDeviceChannel_B
      channels[SUPLA_CHANNELMAXCOUNT];  // Last variable in struct!
} TDS_SuplaRegisterDevice_B;            // ver. >= 2

typedef struct {
  // device -> server

  _supla_int_t LocationID;
  char LocationPWD[SUPLA_LOCATION_PWD_MAXSIZE];  // UTF8

  char GUID[SUPLA_GUID_SIZE];
  char Name[SUPLA_DEVICE_NAME_MAXSIZE];  // UTF8
  char SoftVer[SUPLA_SOFTVER_MAXSIZE];

  char ServerName[SUPLA_SERVER_NAME_MAXSIZE];  // ServerName is equal to Server
                                               // host name/IP

  unsigned char channel_count;
  TDS_SuplaDeviceChannel_B
      channels[SUPLA_CHANNELMAXCOUNT];  // Last variable in struct!
} TDS_SuplaRegisterDevice_C;            // ver. >= 6

typedef struct {
  // device -> server

  char Email[SUPLA_EMAIL_MAXSIZE];  // UTF8
  char AuthKey[SUPLA_AUTHKEY_SIZE];

  char GUID[SUPLA_GUID_SIZE];

  char Name[SUPLA_DEVICE_NAME_MAXSIZE];  // UTF8
  char SoftVer[SUPLA_SOFTVER_MAXSIZE];

  char ServerName[SUPLA_SERVER_NAME_MAXSIZE];

  unsigned char channel_count;
  TDS_SuplaDeviceChannel_B
      channels[SUPLA_CHANNELMAXCOUNT];  // Last variable in struct!
} TDS_SuplaRegisterDevice_D;            // ver. >= 7

typedef struct {
  // device -> server

  char Email[SUPLA_EMAIL_MAXSIZE];  // UTF8
  char AuthKey[SUPLA_AUTHKEY_SIZE];

  char GUID[SUPLA_GUID_SIZE];

  char Name[SUPLA_DEVICE_NAME_MAXSIZE];  // UTF8
  char SoftVer[SUPLA_SOFTVER_MAXSIZE];

  char ServerName[SUPLA_SERVER_NAME_MAXSIZE];

  _supla_int_t Flags;  // SUPLA_DEVICE_FLAG_*
  _supla_int16_t ManufacturerID;
  _supla_int16_t ProductID;

  unsigned char channel_count;
  TDS_SuplaDeviceChannel_C
      channels[SUPLA_CHANNELMAXCOUNT];  // Last variable in struct!
} TDS_SuplaRegisterDevice_E;            // ver. >= 10

typedef struct {
  // device -> server

  char Email[SUPLA_EMAIL_MAXSIZE];  // UTF8
  char AuthKey[SUPLA_AUTHKEY_SIZE];

  char GUID[SUPLA_GUID_SIZE];

  char Name[SUPLA_DEVICE_NAME_MAXSIZE];  // UTF8
  char SoftVer[SUPLA_SOFTVER_MAXSIZE];

  char ServerName[SUPLA_SERVER_NAME_MAXSIZE];

  _supla_int_t Flags;  // SUPLA_DEVICE_FLAG_*
  _supla_int16_t ManufacturerID;
  _supla_int16_t ProductID;

  unsigned char channel_count;
} TDS_SuplaRegisterDeviceHeader;

typedef struct {
  // device -> server

  char Email[SUPLA_EMAIL_MAXSIZE];  // UTF8
  char AuthKey[SUPLA_AUTHKEY_SIZE];

  char GUID[SUPLA_GUID_SIZE];

  char Name[SUPLA_DEVICE_NAME_MAXSIZE];  // UTF8
  char SoftVer[SUPLA_SOFTVER_MAXSIZE];

  char ServerName[SUPLA_SERVER_NAME_MAXSIZE];

  _supla_int_t Flags;  // SUPLA_DEVICE_FLAG_*
  _supla_int16_t ManufacturerID;
  _supla_int16_t ProductID;

  unsigned char channel_count;
  TDS_SuplaDeviceChannel_D
      channels[SUPLA_CHANNELMAXCOUNT];  // Last variable in struct!
} TDS_SuplaRegisterDevice_F;            // ver. >= 23

typedef struct {
  // device -> server

  char Email[SUPLA_EMAIL_MAXSIZE];  // UTF8
  char AuthKey[SUPLA_AUTHKEY_SIZE];

  char GUID[SUPLA_GUID_SIZE];

  char Name[SUPLA_DEVICE_NAME_MAXSIZE];  // UTF8
  char SoftVer[SUPLA_SOFTVER_MAXSIZE];

  char ServerName[SUPLA_SERVER_NAME_MAXSIZE];

  _supla_int_t Flags;  // SUPLA_DEVICE_FLAG_*
  _supla_int16_t ManufacturerID;
  _supla_int16_t ProductID;

  unsigned char channel_count;
  TDS_SuplaDeviceChannel_E
      channels[SUPLA_CHANNELMAXCOUNT];  // Last variable in struct!
} TDS_SuplaRegisterDevice_G;            // ver. >= 25

typedef struct {
  // server -> device

  _supla_int_t result_code;
  unsigned char activity_timeout;
  unsigned char version;
  unsigned char version_min;
} TSD_SuplaRegisterDeviceResult;

#define CHANNEL_REPORT_CHANNEL_REGISTERED (1 << 0)
#define CHANNEL_REPORT_INCORRECT_CHANNEL_TYPE (1 << 1)
#define CHANNEL_REPORT_MAXSIZE 256

typedef struct {
  // server -> device

  _supla_int_t result_code;
  unsigned char activity_timeout;
  unsigned char version;
  unsigned char version_min;

  unsigned _supla_int16_t channel_report_size;
  unsigned char channel_report
      [CHANNEL_REPORT_MAXSIZE];     // One byte per channel. The meaning of the
                                    // bits is determined by CHANNEL_REPORT_*.
} TSD_SuplaRegisterDeviceResult_B;  // ver. >= 25

typedef struct {
  // device -> server

  unsigned char ChannelNumber;
  char value[SUPLA_CHANNELVALUE_SIZE];
} TDS_SuplaDeviceChannelValue;

typedef struct {
  // device -> server

  unsigned char ChannelNumber;
  unsigned char Offline;  // If true, the value variable is ignored.
  char value[SUPLA_CHANNELVALUE_SIZE];
} TDS_SuplaDeviceChannelValue_B;  // v. >= 12

typedef struct {
  // device -> server

  unsigned char ChannelNumber;
  unsigned char
      Offline;  // If true, the ValidityTimeSec and value variables are ignored.
  unsigned _supla_int_t ValidityTimeSec;
  char value[SUPLA_CHANNELVALUE_SIZE];
} TDS_SuplaDeviceChannelValue_C;  // v. >= 12

typedef struct {
  // device -> server

  unsigned char ChannelNumber;
  TSuplaChannelExtendedValue value;     // Last variable in struct!
} TDS_SuplaDeviceChannelExtendedValue;  // v. >= 10

typedef struct {
  // server -> device
  _supla_int_t SenderID;
  unsigned char ChannelNumber;
  union {
    unsigned _supla_int_t DurationMS;
    unsigned _supla_int_t DurationSec;  // ver. >= 21. Applies to HVAC
  };

  char value[SUPLA_CHANNELVALUE_SIZE];
} TSD_SuplaChannelNewValue;

typedef struct {
  // server -> device
  _supla_int_t SenderID;
  _supla_int_t GroupID;
  unsigned char EOL;  // End Of List
  unsigned char ChannelNumber;
  union {
    unsigned _supla_int_t DurationMS;
    unsigned _supla_int_t DurationSec;  // ver. >= 21. Applies to HVAC
  };
  char value[SUPLA_CHANNELVALUE_SIZE];
} TSD_SuplaChannelGroupNewValue;  // v. >= 13

typedef struct {
  // device -> server
  unsigned char ChannelNumber;
  _supla_int_t SenderID;
  char Success;
} TDS_SuplaChannelNewValueResult;

typedef struct {
  // server -> client

  char EOL;  // End Of List
  _supla_int_t Id;
  char online;

  TSuplaChannelValue value;
} TSC_SuplaChannelValue;

typedef struct {
  // server -> client

  char EOL;  // End Of List
  _supla_int_t Id;
  char online;

  TSuplaChannelValue_B value;
} TSC_SuplaChannelValue_B;  //  ver. >= 15

typedef struct {
  // server -> client
  _supla_int_t Id;

  TSuplaChannelExtendedValue value;  // Last variable in struct!
} TSC_SuplaChannelExtendedValue;

typedef struct {
  // server -> client

  _supla_int_t count;
  _supla_int_t total_left;

  TSC_SuplaChannelValue
      items[SUPLA_CHANNELVALUE_PACK_MAXCOUNT];  // Last variable in struct!
} TSC_SuplaChannelValuePack;                    // ver. >= 9

typedef struct {
  // server -> client

  _supla_int_t count;
  _supla_int_t total_left;

  TSC_SuplaChannelValue_B
      items[SUPLA_CHANNELVALUE_PACK_MAXCOUNT];  // Last variable in struct!
} TSC_SuplaChannelValuePack_B;                  // ver. >= 15

typedef struct {
  // server -> client

  _supla_int_t count;
  _supla_int_t total_left;
  unsigned _supla_int_t pack_size;

  char pack[SUPLA_CHANNELEXTENDEDVALUE_PACK_MAXDATASIZE];  // Last variable in
                                                           // struct!
} TSC_SuplaChannelExtendedValuePack;                       // ver. >= 10

typedef struct {
  // server -> client
  char EOL;  // End Of List

  _supla_int_t Id;
  _supla_int_t LocationID;
  _supla_int_t Func;
  char online;

  TSuplaChannelValue value;

  unsigned _supla_int_t
      CaptionSize;  // including the terminating null byte ('\0')
  char Caption[SUPLA_CHANNEL_CAPTION_MAXSIZE];  // Last variable in struct!
} TSC_SuplaChannel;

typedef struct {
  // server -> client

  _supla_int_t count;
  _supla_int_t total_left;
  TSC_SuplaChannel
      items[SUPLA_CHANNELPACK_MAXCOUNT];  // Last variable in struct!
} TSC_SuplaChannelPack;

typedef struct {
  // server -> client
  char EOL;  // End Of List

  _supla_int_t Id;
  _supla_int_t LocationID;
  _supla_int_t Func;
  _supla_int_t AltIcon;
  unsigned _supla_int_t Flags;
  unsigned char ProtocolVersion;
  char online;

  TSuplaChannelValue value;

  unsigned _supla_int_t
      CaptionSize;  // including the terminating null byte ('\0')
  char Caption[SUPLA_CHANNEL_CAPTION_MAXSIZE];  // Last variable in struct!
} TSC_SuplaChannel_B;                           // ver. >= 8

typedef struct {
  // server -> client

  _supla_int_t count;
  _supla_int_t total_left;
  TSC_SuplaChannel_B
      items[SUPLA_CHANNELPACK_MAXCOUNT];  // Last variable in struct!
} TSC_SuplaChannelPack_B;                 // ver. >= 8

typedef struct {
  // server -> client
  char EOL;  // End Of List

  _supla_int_t Id;
  _supla_int_t DeviceID;
  _supla_int_t LocationID;
  _supla_int_t Type;
  _supla_int_t Func;
  _supla_int_t AltIcon;
  _supla_int_t UserIcon;
  _supla_int16_t ManufacturerID;
  _supla_int16_t ProductID;

  unsigned _supla_int_t Flags;
  unsigned char ProtocolVersion;
  char online;

  TSuplaChannelValue value;

  unsigned _supla_int_t
      CaptionSize;  // including the terminating null byte ('\0')
  char Caption[SUPLA_CHANNEL_CAPTION_MAXSIZE];  // Last variable in struct!
} TSC_SuplaChannel_C;                           // ver. >= 10

typedef struct {
  // server -> client
  char EOL;  // End Of List

  _supla_int_t Id;
  _supla_int_t DeviceID;
  _supla_int_t LocationID;
  _supla_int_t Type;
  _supla_int_t Func;
  _supla_int_t AltIcon;
  _supla_int_t UserIcon;
  _supla_int16_t ManufacturerID;
  _supla_int16_t ProductID;

  unsigned _supla_int_t Flags;
  unsigned char ProtocolVersion;
  char online;

  TSuplaChannelValue_B value;

  unsigned _supla_int_t
      CaptionSize;  // including the terminating null byte ('\0')
  char Caption[SUPLA_CHANNEL_CAPTION_MAXSIZE];  // Last variable in struct!
} TSC_SuplaChannel_D;                           // ver. >= 15

typedef struct {
  // server -> client
  char EOL;  // End Of List

  _supla_int_t Id;
  _supla_int_t DeviceID;
  _supla_int_t LocationID;
  _supla_int_t Type;
  _supla_int_t Func;
  _supla_int_t AltIcon;
  _supla_int_t UserIcon;
  _supla_int16_t ManufacturerID;
  _supla_int16_t ProductID;
  unsigned _supla_int_t DefaultConfigCRC32;

  unsigned _supla_int64_t Flags;
  unsigned char ProtocolVersion;
  char online;

  TSuplaChannelValue_B value;

  unsigned _supla_int_t
      CaptionSize;  // including the terminating null byte ('\0')
  char Caption[SUPLA_CHANNEL_CAPTION_MAXSIZE];  // Last variable in struct!
} TSC_SuplaChannel_E;                           // ver. >= 23

typedef struct {
  // server -> client

  _supla_int_t count;
  _supla_int_t total_left;
  TSC_SuplaChannel_C
      items[SUPLA_CHANNELPACK_MAXCOUNT];  // Last variable in struct!
} TSC_SuplaChannelPack_C;                 // ver. >= 10

typedef struct {
  // server -> client

  _supla_int_t count;
  _supla_int_t total_left;
  TSC_SuplaChannel_D
      items[SUPLA_CHANNELPACK_MAXCOUNT];  // Last variable in struct!
} TSC_SuplaChannelPack_D;                 // ver. >= 15

typedef struct {
  // server -> client

  _supla_int_t count;
  _supla_int_t total_left;
  TSC_SuplaChannel_E
      items[SUPLA_CHANNELPACK_MAXCOUNT];  // Last variable in struct!
} TSC_SuplaChannelPack_E;                 // ver. >= 23

typedef struct {
  // server -> client
  char EOL;  // End Of List

  _supla_int_t Id;
  _supla_int_t LocationID;
  _supla_int_t Func;
  _supla_int_t AltIcon;
  unsigned _supla_int_t Flags;

  unsigned _supla_int_t
      CaptionSize;  // including the terminating null byte ('\0')
  char Caption[SUPLA_CHANNELGROUP_CAPTION_MAXSIZE];  // Last variable in struct!
} TSC_SuplaChannelGroup;                             // ver. >= 9

typedef struct {
  // server -> client
  char EOL;  // End Of List

  _supla_int_t Id;
  _supla_int_t LocationID;
  _supla_int_t Func;
  _supla_int_t AltIcon;
  _supla_int_t UserIcon;
  unsigned _supla_int_t Flags;

  unsigned _supla_int_t
      CaptionSize;  // including the terminating null byte ('\0')
  char Caption[SUPLA_CHANNELGROUP_CAPTION_MAXSIZE];  // Last variable in struct!
} TSC_SuplaChannelGroup_B;                           // ver. >= 10

typedef struct {
  // server -> client

  _supla_int_t count;
  _supla_int_t total_left;
  TSC_SuplaChannelGroup
      items[SUPLA_CHANNELGROUP_PACK_MAXCOUNT];  // Last variable in struct!
} TSC_SuplaChannelGroupPack;                    // ver. >= 9

typedef struct {
  // server -> client

  _supla_int_t count;
  _supla_int_t total_left;
  TSC_SuplaChannelGroup_B
      items[SUPLA_CHANNELGROUP_PACK_MAXCOUNT];  // Last variable in struct!
} TSC_SuplaChannelGroupPack_B;                  // ver. >= 10

typedef struct {
  // server -> client
  char EOL;  // End Of List

  _supla_int_t ChannelGroupID;
  _supla_int_t ChannelID;
} TSC_SuplaChannelGroupRelation;  // ver. >= 9

typedef struct {
  // server -> client

  _supla_int_t count;
  _supla_int_t total_left;
  TSC_SuplaChannelGroupRelation
      items[SUPLA_CHANNELGROUP_RELATION_PACK_MAXCOUNT];  // Last variable in
                                                         // struct!
} TSC_SuplaChannelGroupRelationPack;                     // ver. >= 9

#define CHANNEL_RELATION_TYPE_DEFAULT 0
#define CHANNEL_RELATION_TYPE_OPENING_SENSOR 1
#define CHANNEL_RELATION_TYPE_PARTIAL_OPENING_SENSOR 2
#define CHANNEL_RELATION_TYPE_METER 3
#define CHANNEL_RELATION_TYPE_MAIN_TERMOMETER 4
#define CHANNEL_RELATION_TYPE_AUX_THERMOMETER_FLOOR 5
#define CHANNEL_RELATION_TYPE_AUX_THERMOMETER_WATER 6
#define CHANNEL_RELATION_TYPE_AUX_THERMOMETER_GENERIC_HEATER 7
#define CHANNEL_RELATION_TYPE_AUX_THERMOMETER_GENERIC_COOLER 8

#define CHANNEL_RELATION_TYPE_MASTER_THERMOSTAT 20
#define CHANNEL_RELATION_TYPE_HEAT_OR_COLD_SOURCE_SWITCH 21
#define CHANNEL_RELATION_TYPE_PUMP_SWITCH 22

typedef struct {
  char EOL;  // End Of List
  _supla_int_t Id;
  _supla_int_t ParentId;
  _supla_int16_t Type;       // CHANNEL_RELATION_TYPE_
} TSC_SuplaChannelRelation;  //  ver. >= 21

typedef struct {
  _supla_int_t count;
  _supla_int_t total_left;
  TSC_SuplaChannelRelation
      items[SUPLA_CHANNEL_RELATION_PACK_MAXCOUNT];  // Last variable in struct!
} TSC_SuplaChannelRelationPack;

typedef struct {
  // server -> client
  char EOL;  // End Of List

  _supla_int_t Id;
  _supla_int_t LocationId;
  _supla_int_t AltIcon;
  _supla_int_t UserIcon;

  unsigned _supla_int16_t
      CaptionSize;  // including the terminating null byte ('\0')
  char Caption[SUPLA_SCENE_CAPTION_MAXSIZE];  // Last variable in struct!
} TSC_SuplaScene;                             // ver. >= 18

typedef struct {
  // server -> client

  _supla_int_t count;
  _supla_int_t total_left;
  TSC_SuplaScene items[SUPLA_SCENE_PACK_MAXCOUNT];  // Last variable in struct!
} TSC_SuplaScenePack;                               // ver. >= 18

typedef struct {
  // server -> client
  char EOL;  // End Of List

  _supla_int_t SceneId;
  unsigned _supla_int_t MillisecondsFromStart;
  unsigned _supla_int_t MillisecondsLeft;

  _supla_int_t InitiatorId;
  unsigned _supla_int16_t
      InitiatorNameSize;  // including the terminating null byte ('\0')
  char InitiatorName[SUPLA_INITIATOR_NAME_MAXSIZE];  // Last variable in struct!
} TSC_SuplaSceneState;                               // ver. >= 18

typedef struct {
  // server -> client

  _supla_int_t count;
  _supla_int_t total_left;
  TSC_SuplaSceneState
      items[SUPLA_SCENE_STATE_PACK_MAXCOUNT];  // Last variable in struct!
} TSC_SuplaSceneStatePack;                     // ver. >= 18

#define ACTION_SUBJECT_TYPE_UNKNOWN 0
#define ACTION_SUBJECT_TYPE_CHANNEL 1
#define ACTION_SUBJECT_TYPE_CHANNEL_GROUP 2
#define ACTION_SUBJECT_TYPE_SCENE 3
#define ACTION_SUBJECT_TYPE_SCHEDULE 4

#define ACTION_OPEN 10
#define ACTION_CLOSE 20
#define ACTION_SHUT 30
#define ACTION_REVEAL 40
#define ACTION_REVEAL_PARTIALLY 50
#define ACTION_SHUT_PARTIALLY 51
#define ACTION_TURN_ON 60
#define ACTION_TURN_ON_WITH_DURATION 61
#define ACTION_TURN_OFF 70
#define ACTION_TURN_OFF_WITH_DURATION 71
#define ACTION_SET_RGBW_PARAMETERS 80
#define ACTION_OPEN_CLOSE 90
#define ACTION_STOP 100
#define ACTION_TOGGLE 110
#define ACTION_UP_OR_STOP 140
#define ACTION_DOWN_OR_STOP 150
#define ACTION_STEP_BY_STEP 160
#define ACTION_UP 170
#define ACTION_DOWN 180
#define ACTION_ENABLE 200
#define ACTION_DISABLE 210
#define ACTION_SEND 220
#define ACTION_HVAC_SET_PARAMETERS 230
#define ACTION_HVAC_SWITCH_TO_PROGRAM_MODE 231
#define ACTION_HVAC_SWITCH_TO_MANUAL_MODE 232
#define ACTION_HVAC_SET_TEMPERATURES 233
#define ACTION_HVAC_SET_TEMPERATURE 234
#define ACTION_READ 1000
#define ACTION_SET 2000
#define ACTION_EXECUTE 3000
#define ACTION_INTERRUPT 3001
#define ACTION_INTERRUPT_AND_EXECUTE 3002
#define ACTION_COPY 10100
#define ACTION_FORWARD_OUTSIDE 10000

#define SSP_FLAG_PERCENTAGE_AS_DELTA (1 << 0)
#define SSP_FLAG_TILT_AS_DELTA (1 << 1)

typedef struct {
  char Percentage;
  unsigned char Flags;  // SSP_FLAG_
  signed char Tilt;
  char Reserved[13];
} TAction_ShadingSystem_Parameters;  // ver. >= 19

typedef struct {
  char Brightness;              // -1 == Ignore
  char ColorBrightness;         // -1 == Ignore
  unsigned _supla_int_t Color;  // 0 == Ignore
  char ColorRandom;
  char OnOff;
  char Reserved[8];
} TAction_RGBW_Parameters;  // ver. >= 19

typedef struct {
  unsigned _supla_int_t DurationSec;
  unsigned char Mode;  // for HVAC: SUPLA_HVAC_MODE_
  _supla_int16_t
      SetpointTemperatureHeat;  // * 0.01 Celcius degree - used for heating
  _supla_int16_t
      SetpointTemperatureCool;    // * 0.01 - Celcius degree used for coolingx
  unsigned _supla_int16_t Flags;  // SUPLA_HVAC_VALUE_FLAG_
} TAction_HVAC_Parameters;

typedef struct {
  _supla_int_t ActionId;
  _supla_int_t SubjectId;
  unsigned char SubjectType;
  unsigned _supla_int16_t ParamSize;
  char Param[SUPLA_ACTION_PARAM_MAXSIZE];
} TCS_Action;  // ver. >= 19

typedef struct {
  _supla_int_t AccessID;
  char AccessIDpwd[SUPLA_ACCESSID_PWD_MAXSIZE];  // UTF8
  char Email[SUPLA_EMAIL_MAXSIZE];               // UTF8
  char AuthKey[SUPLA_AUTHKEY_SIZE];
  char GUID[SUPLA_GUID_SIZE];
  char ServerName[SUPLA_SERVER_NAME_MAXSIZE];
} TCS_ClientAuthorizationDetails;  // ver. >= 19

typedef struct {
  TCS_ClientAuthorizationDetails Auth;
  TCS_Action Action;
} TCS_ActionWithAuth;  // ver. >= 19

typedef struct {
  unsigned char ResultCode;
  _supla_int_t ActionId;
  _supla_int_t SubjectId;
  _supla_int_t SubjectType;
} TSC_ActionExecutionResult;  // ver. >= 19

typedef struct {
  // server -> client
  unsigned char ResultCode;
  _supla_int_t ChannelId;  // Alias SubjectId
  _supla_int_t Function;

  TSuplaChannelValue_B Value;
  TSuplaChannelExtendedValue ExtendedValue;  // Last variable in struct!
} TSC_GetChannelValueResult;                 // ver. >= 19;

typedef struct {
  TCS_ClientAuthorizationDetails Auth;
  _supla_int_t ChannelId;
} TCS_GetChannelValueWithAuth;  // ver. >= 19

typedef struct {
  // client -> server

  _supla_int_t AccessID;
  char AccessIDpwd[SUPLA_ACCESSID_PWD_MAXSIZE];  // UTF8

  char GUID[SUPLA_GUID_SIZE];
  char Name[SUPLA_CLIENT_NAME_MAXSIZE];  // UTF8
  char SoftVer[SUPLA_SOFTVER_MAXSIZE];
} TCS_SuplaRegisterClient;

typedef struct {
  // client -> server

  _supla_int_t AccessID;
  char AccessIDpwd[SUPLA_ACCESSID_PWD_MAXSIZE];  // UTF8

  char GUID[SUPLA_GUID_SIZE];
  char Name[SUPLA_CLIENT_NAME_MAXSIZE];  // UTF8
  char SoftVer[SUPLA_SOFTVER_MAXSIZE];

  char ServerName[SUPLA_SERVER_NAME_MAXSIZE];
} TCS_SuplaRegisterClient_B;  // ver. >= 6

typedef struct {
  // client -> server

  char Email[SUPLA_EMAIL_MAXSIZE];  // UTF8
  char AuthKey[SUPLA_AUTHKEY_SIZE];

  char GUID[SUPLA_GUID_SIZE];
  char Name[SUPLA_CLIENT_NAME_MAXSIZE];  // UTF8
  char SoftVer[SUPLA_SOFTVER_MAXSIZE];

  char ServerName[SUPLA_SERVER_NAME_MAXSIZE];
} TCS_SuplaRegisterClient_C;  // ver. >= 7

typedef struct {
  // client -> server

  char Email[SUPLA_EMAIL_MAXSIZE];        // UTF8
  char Password[SUPLA_PASSWORD_MAXSIZE];  // Optional - UTF8
  char AuthKey[SUPLA_AUTHKEY_SIZE];

  char GUID[SUPLA_GUID_SIZE];
  char Name[SUPLA_CLIENT_NAME_MAXSIZE];  // UTF8
  char SoftVer[SUPLA_SOFTVER_MAXSIZE];

  char ServerName[SUPLA_SERVER_NAME_MAXSIZE];
} TCS_SuplaRegisterClient_D;  // ver. >= 12

typedef struct {
  // server -> client

  _supla_int_t result_code;
  _supla_int_t ClientID;
  _supla_int_t LocationCount;
  _supla_int_t ChannelCount;
  unsigned char activity_timeout;
  unsigned char version;
  unsigned char version_min;
} TSC_SuplaRegisterClientResult;

typedef struct {
  // server -> client

  _supla_int_t result_code;
  _supla_int_t ClientID;
  _supla_int_t LocationCount;
  _supla_int_t ChannelCount;
  _supla_int_t ChannelGroupCount;
  _supla_int_t Flags;
  unsigned char activity_timeout;
  unsigned char version;
  unsigned char version_min;
} TSC_SuplaRegisterClientResult_B;  // ver. >= 9

typedef struct {
  // server -> client

  _supla_int_t result_code;
  _supla_int_t ClientID;
  _supla_int_t LocationCount;
  _supla_int_t ChannelCount;
  _supla_int_t ChannelGroupCount;
  _supla_int_t Flags;
  unsigned char activity_timeout;
  unsigned char version;
  unsigned char version_min;
  unsigned _supla_int_t serverUnixTimestamp;  // current server time
} TSC_SuplaRegisterClientResult_C;            // ver. >= 17

typedef struct {
  // server -> client

  _supla_int_t result_code;
  _supla_int_t ClientID;
  _supla_int16_t LocationCount;
  _supla_int16_t ChannelCount;
  _supla_int16_t ChannelGroupCount;
  _supla_int16_t SceneCount;
  _supla_int_t Flags;
  unsigned char activity_timeout;
  unsigned char version;
  unsigned char version_min;
  unsigned _supla_int_t serverUnixTimestamp;  // current server time
} TSC_SuplaRegisterClientResult_D;            // ver. >= 19

typedef struct {
  // client -> server
  unsigned char ChannelId;
  char value[SUPLA_CHANNELVALUE_SIZE];
} TCS_SuplaChannelNewValue;  // Deprecated

typedef struct {
  // client -> server
  _supla_int_t ChannelId;
  char value[SUPLA_CHANNELVALUE_SIZE];
} TCS_SuplaChannelNewValue_B;

typedef struct {
  // client -> server
  _supla_int_t Id;
  char Target;  // SUPLA_TARGET_
  char value[SUPLA_CHANNELVALUE_SIZE];
} TCS_SuplaNewValue;  // ver. >= 9

typedef struct {
  // server -> client
  _supla_int_t Event;
  _supla_int_t ChannelID;
  unsigned _supla_int_t DurationMS;

  _supla_int_t SenderID;
  unsigned _supla_int_t
      SenderNameSize;  // including the terminating null byte ('\0')
  char
      SenderName[SUPLA_SENDER_NAME_MAXSIZE];  // Last variable in struct! | UTF8
} TSC_SuplaEvent;

typedef struct {
  char Platform;

  _supla_int_t Param1;
  _supla_int_t Param2;
  _supla_int_t Param3;
  _supla_int_t Param4;
} TDS_FirmwareUpdateParams;

typedef struct {
  char available_protocols;
  char host[SUPLA_URL_HOST_MAXSIZE];
  _supla_int_t port;
  char path[SUPLA_URL_PATH_MAXSIZE];
} TSD_FirmwareUpdate_Url;

typedef struct {
  char exists;
  TSD_FirmwareUpdate_Url url;
} TSD_FirmwareUpdate_UrlResult;

typedef struct {
  unsigned _supla_int_t client_timestamp;    // time >= now == enabled
  unsigned _supla_int_t iodevice_timestamp;  // time >= now == enabled
} TSDC_RegistrationEnabled;

typedef struct {
  unsigned _supla_int_t ExpiresIn;
  unsigned _supla_int_t
      TokenSize;  // including the terminating null byte ('\0')
  char Token[SUPLA_OAUTH_TOKEN_MAXSIZE];  // Last variable in struct!
} TSC_OAuthToken;                         // ver. >= 10

typedef struct {
  unsigned char ResultCode;
  TSC_OAuthToken Token;
} TSC_OAuthTokenRequestResult;  // ver. >= 10

typedef struct {
  // 3 phases
  unsigned _supla_int16_t freq;        // * 0.01 Hz
  unsigned _supla_int16_t voltage[3];  // * 0.01 V
  unsigned _supla_int16_t
      current[3];  // * 0.001A (0.01A WHEN EM_VAR_CURRENT_OVER_65A)
  _supla_int_t
      power_active[3];  // * 0.00001W (0.01kW WHEN EM_VAR_POWER_ACTIVE_KW)
  _supla_int_t power_reactive[3];  // * 0.00001var (0.01kvar WHEN
                                   // EM_VAR_POWER_REACTIVE_KVAR)
  _supla_int_t power_apparent[3];  // * 0.00001VA (0.01kVA WHEN
                                   // EM_VAR_POWER_APPARENT_KVA)
  _supla_int16_t power_factor[3];  // * 0.001
  _supla_int16_t phase_angle[3];   // * 0.1 degree
} TElectricityMeter_Measurement;   // v. >= 10

#define EM_VAR_FREQ 0x0001
#define EM_VAR_VOLTAGE 0x0002
#define EM_VAR_CURRENT 0x0004
#define EM_VAR_POWER_ACTIVE 0x0008
#define EM_VAR_POWER_REACTIVE 0x0010
#define EM_VAR_POWER_APPARENT 0x0020
#define EM_VAR_POWER_FACTOR 0x0040
#define EM_VAR_PHASE_ANGLE 0x0080
#define EM_VAR_FORWARD_ACTIVE_ENERGY 0x0100
#define EM_VAR_REVERSE_ACTIVE_ENERGY 0x0200
#define EM_VAR_FORWARD_REACTIVE_ENERGY 0x0400
#define EM_VAR_REVERSE_REACTIVE_ENERGY 0x0800
#define EM_VAR_CURRENT_OVER_65A 0x1000
#define EM_VAR_FORWARD_ACTIVE_ENERGY_BALANCED 0x2000
#define EM_VAR_REVERSE_ACTIVE_ENERGY_BALANCED 0x4000

#define EM_VAR_VOLTAGE_PHASE_ANGLE_12 0x10000  // ver. >= 25
#define EM_VAR_VOLTAGE_PHASE_ANGLE_13 0x20000  // ver. >= 25
#define EM_VAR_VOLTAGE_PHASE_SEQUENCE 0x40000  // ver. >= 25
#define EM_VAR_CURRENT_PHASE_SEQUENCE 0x80000  // ver. >= 25

#define EM_VAR_POWER_ACTIVE_KW 0x100000
#define EM_VAR_POWER_REACTIVE_KVAR 0x200000
#define EM_VAR_POWER_APPARENT_KVA 0x400000

#define EM_PHASE_SEQUENCE_VOLTAGE 0x01
#define EM_PHASE_SEQUENCE_CURRENT 0x02

#if defined(__AVR__) || defined(ESP8266) || defined(ESP32) || \
    defined(ESP_PLATFORM) || defined(ARDUINO) || defined(SUPLA_DEVICE)
#define EM_MEASUREMENT_COUNT 1
#else
#define EM_MEASUREMENT_COUNT 5
#endif

#ifdef USE_DEPRECATED_EMEV_V1
// [IODevice->Server->Client]
typedef struct {
  unsigned _supla_int64_t total_forward_active_energy[3];    // * 0.00001 kWh
  unsigned _supla_int64_t total_reverse_active_energy[3];    // * 0.00001 kWh
  unsigned _supla_int64_t total_forward_reactive_energy[3];  // * 0.00001 kvarh
  unsigned _supla_int64_t total_reverse_reactive_energy[3];  // * 0.00001 kvarh

  // The price per unit, total cost and currency is overwritten by the server
  // total_cost == SUM(total_forward_active_energy[n] * price_per_unit
  _supla_int_t total_cost;      // * 0.01
  _supla_int_t price_per_unit;  // * 0.0001
  // Currency Code A https://www.nationsonline.org/oneworld/currencies.htm
  char currency[3];

  _supla_int_t measured_values;
  _supla_int_t period;  // Approximate period between measurements in seconds
  _supla_int_t m_count;
  TElectricityMeter_Measurement m[EM_MEASUREMENT_COUNT];  // Last variable in
                                                          // struct!
} TElectricityMeter_ExtendedValue;                        // v. >= 10
#endif /*USE_DEPRECATED_EMEV_V1*/

#ifdef USE_DEPRECATED_EMEV_V2
// [IODevice->Server->Client]
typedef struct {
  unsigned _supla_int64_t total_forward_active_energy[3];    // * 0.00001 kWh
  unsigned _supla_int64_t total_reverse_active_energy[3];    // * 0.00001 kWh
  unsigned _supla_int64_t total_forward_reactive_energy[3];  // * 0.00001 kvarh
  unsigned _supla_int64_t total_reverse_reactive_energy[3];  // * 0.00001 kvarh
  unsigned _supla_int64_t
      total_forward_active_energy_balanced;  // * 0.00001 kWh
                                             // Vector phase-to-phase balancing
  unsigned _supla_int64_t
      total_reverse_active_energy_balanced;  // * 0.00001 kWh
                                             // Vector phase-to-phase balancing

  // The price per unit, total cost and currency is overwritten by the server
  // total_cost == SUM(total_forward_active_energy[n] * price_per_unit
  _supla_int_t total_cost;           // * 0.01
  _supla_int_t total_cost_balanced;  // * 0.01
  _supla_int_t price_per_unit;       // * 0.0001
  // Currency Code A https://www.nationsonline.org/oneworld/currencies.htm
  char currency[3];

  _supla_int_t measured_values;
  _supla_int_t period;  // Approximate period between measurements in seconds
  _supla_int_t m_count;
  TElectricityMeter_Measurement m[EM_MEASUREMENT_COUNT];  // Last variable in
                                                          // struct!
} TElectricityMeter_ExtendedValue_V2;                     // v. >= 12
#endif /*USE_DEPRECATED_EMEV_V2*/

// [IODevice->Server->Client]
typedef struct {
  unsigned _supla_int64_t total_forward_active_energy[3];    // * 0.00001 kWh
  unsigned _supla_int64_t total_reverse_active_energy[3];    // * 0.00001 kWh
  unsigned _supla_int64_t total_forward_reactive_energy[3];  // * 0.00001 kvarh
  unsigned _supla_int64_t total_reverse_reactive_energy[3];  // * 0.00001 kvarh
  unsigned _supla_int64_t
      total_forward_active_energy_balanced;  // * 0.00001 kWh
                                             // Vector phase-to-phase balancing
  unsigned _supla_int64_t
      total_reverse_active_energy_balanced;  // * 0.00001 kWh
                                             // Vector phase-to-phase balancing

  // Voltage phase angle between phase 1 and 2
  unsigned _supla_int16_t voltage_phase_angle_12;  // * 0.1 degree, 0..360
  // Voltage phase angle between phase 1 and 3
  unsigned _supla_int16_t voltage_phase_angle_13;  // * 0.1 degree, 0..360
  unsigned char phase_sequence;  // bit 0x1 - voltage, bit 0x2 current
                                 // EM_PHASE_SEQUENCE_*
                                 // bit value: 0 - 123 (clockwise)
                                 // bit value: 1 - 132 (counter-clockwise)

  // The price per unit, total cost and currency is overwritten by the server
  // total_cost == SUM(total_forward_active_energy[n] * price_per_unit
  _supla_int_t total_cost;           // * 0.01
  _supla_int_t total_cost_balanced;  // * 0.01
  _supla_int_t price_per_unit;       // * 0.0001
  // Currency Code A https://www.nationsonline.org/oneworld/currencies.htm
  char currency[3];

  _supla_int_t measured_values;
  _supla_int_t period;  // Approximate period between measurements in seconds
  _supla_int_t m_count;
  TElectricityMeter_Measurement m[EM_MEASUREMENT_COUNT];  // Last variable in
                                                          // struct!
} TElectricityMeter_ExtendedValue_V3;                     // v. >= 25

#define EM_VALUE_FLAG_PHASE1_ON 0x01
#define EM_VALUE_FLAG_PHASE2_ON 0x02
#define EM_VALUE_FLAG_PHASE3_ON 0x04

// [IODevice->Server->Client]
typedef struct {
  char flags;
  unsigned _supla_int_t total_forward_active_energy;  // * 0.01 kW
} TElectricityMeter_Value;                            // v. >= 10

typedef struct {
  // The price per unit, total cost and currency is overwritten by the server
  // total_cost = calculated_value * price_per_unit
  _supla_int_t total_cost;      // * 0.01
  _supla_int_t price_per_unit;  // * 0.0001
  // Currency Code A https://www.nationsonline.org/oneworld/currencies.htm
  char currency[3];
  char custom_unit[9];  // UTF8 including the terminating null byte ('\0')

  _supla_int_t impulses_per_unit;
  unsigned _supla_int64_t counter;
  _supla_int64_t calculated_value;   // * 0.001
} TSC_ImpulseCounter_ExtendedValue;  // v. >= 10

typedef struct {
  unsigned _supla_int64_t counter;
} TDS_ImpulseCounter_Value;

#define RS_VALUE_FLAG_TILT_IS_SET 0x1
#define RS_VALUE_FLAG_CALIBRATION_FAILED 0x2
#define RS_VALUE_FLAG_CALIBRATION_LOST 0x4
#define RS_VALUE_FLAG_MOTOR_PROBLEM 0x8
#define RS_VALUE_FLAG_CALIBRATION_IN_PROGRESS 0x10

#define SUPLA_TILT_CONTROL_TYPE_UNKNOWN 0
#define SUPLA_TILT_CONTROL_TYPE_STANDS_IN_POSITION_WHILE_TILTING 1
#define SUPLA_TILT_CONTROL_TYPE_CHANGES_POSITION_WHILE_TILTING 2
#define SUPLA_TILT_CONTROL_TYPE_TILTS_ONLY_WHEN_FULLY_CLOSED 3

// Roller shutter channel value payload
// Device -> Server -> Client
typedef struct {
  signed char position;  // -1 == calibration. -1, 0% (open) - 100% (closed) DSC
  char reserved1;
  signed char bottom_position;  // Percentage points to the windowsill, SC
  _supla_int16_t flags;         // DSC
  char reserved2;
  char reserved3;
  char reserved4;
} TDSC_RollerShutterValue;

typedef TDSC_RollerShutterValue TDSC_TerraceAwningValue;
typedef TDSC_RollerShutterValue TDSC_ProjectorScreen;
typedef TDSC_RollerShutterValue TDSC_CurtainValue;
typedef TDSC_RollerShutterValue TDSC_RollerGarageDoor;

// Roller shutter channel value payload
// Client -> Server -> Device
typedef struct {
  signed char position;  // 0 - STOP
                         // 1 - DOWN (CLOSE)
                         // 2 - UP (OPEN)
                         // 3 - DOWN_OR_STOP (CLOSE_OR_STOP)
                         // 4 - UP_OR_STOP (OPEN_OR_STOP)
                         // 5 - STEP_BY_STEP
                         // 10-110 - target position + 10 (0 open, 100 closed)
  char reserved[7];
} TCSD_RollerShutterValue;

typedef TCSD_RollerShutterValue TCSD_TerraceAwningValue;
typedef TCSD_RollerShutterValue TCSD_ProjectorScreen;
typedef TCSD_RollerShutterValue TCSD_CurtainValue;
typedef TCSD_RollerShutterValue TCSD_RollerGarageDoor;

// Facade blind channel value payload
// Device -> Server -> Client
typedef struct {
  signed char position;  // -1 == calibration. -1, 0% (open) - 100% (closed) DSC
  signed char tilt;      // -1 == not used/calibration, -1, 0% - 100%, DSC
  char reserved;
  _supla_int16_t flags;  // DSC
  char reserved2[3];
} TDSC_FacadeBlindValue;

typedef TDSC_FacadeBlindValue TDSC_VerticalBlindValue;

// Facade blind channel value payload
// Client -> Server -> Device
typedef struct {
  signed char position;  // -1 - not set (actual behavior is device specific)
                         // 0 - STOP
                         // 1 - DOWN (CLOSE)
                         // 2 - UP (OPEN)
                         // 3 - DOWN_OR_STOP (CLOSE_OR_STOP)
                         // 4 - UP_OR_STOP (OPEN_OR_STOP)
                         // 5 - STEP_BY_STEP
                         // 10-110 - target position + 10 (0 open, 100 closed)
  signed char tilt;      // -1 - not set (actual behavior is device specific)
                         // 10-110 - target position + 10
  char reserved[6];
} TCSD_FacadeBlindValue;

typedef TCSD_FacadeBlindValue TCSD_VerticalBlindValue;

typedef struct {
  unsigned _supla_int64_t calculated_value;  // * 0.001
} TSC_ImpulseCounter_Value;                  // v. >= 10

typedef struct {
  char Email[SUPLA_EMAIL_MAXSIZE];        // UTF8
  char Password[SUPLA_PASSWORD_MAXSIZE];  // UTF8
} TCS_SuperUserAuthorizationRequest;      // v. >= 10

typedef struct {
  _supla_int_t Result;
} TSC_SuperUserAuthorizationResult;  // v. >= 10

#define SUPLA_CALCFG_RESULT_FALSE 0                          // ver. >= 12
#define SUPLA_CALCFG_RESULT_TRUE 1                           // ver. >= 12
#define SUPLA_CALCFG_RESULT_DONE 2                           // ver. >= 12
#define SUPLA_CALCFG_RESULT_IN_PROGRESS 3                    // ver. >= 12
#define SUPLA_CALCFG_RESULT_NODE_FOUND 4                     // ver. >= 12
#define SUPLA_CALCFG_RESULT_SENDER_CONFLICT 100              // ver. >= 12
#define SUPLA_CALCFG_RESULT_TIMEOUT 101                      // ver. >= 12
#define SUPLA_CALCFG_RESULT_NOT_SUPPORTED 102                // ver. >= 12
#define SUPLA_CALCFG_RESULT_ID_NOT_EXISTS 103                // ver. >= 12
#define SUPLA_CALCFG_RESULT_UNAUTHORIZED 104                 // ver. >= 12
#define SUPLA_CALCFG_RESULT_DEBUG 105                        // ver. >= 12
#define SUPLA_CALCFG_RESULT_NOT_SUPPORTED_IN_SLAVE_MODE 106  // ver. >= 12

#define SUPLA_CALCFG_CMD_GET_CHANNEL_FUNCLIST 1000        // v. >= 11
#define SUPLA_CALCFG_CMD_CANCEL_ALL_CLIENT_COMMANDS 1010  // v. >= 12
#define SUPLA_CALCFG_CMD_ZWAVE_RESET_AND_CLEAR 2000       // v. >= 12
#define SUPLA_CALCFG_CMD_ZWAVE_ADD_NODE 2010              // v. >= 12
#define SUPLA_CALCFG_CMD_ZWAVE_REMOVE_NODE 2020           // v. >= 12
#define SUPLA_CALCFG_CMD_ZWAVE_GET_NODE_LIST 2030         // v. >= 12
#define SUPLA_CALCFG_CMD_ZWAVE_GET_ASSIGNED_NODE_ID 2040  // v. >= 12
#define SUPLA_CALCFG_CMD_ZWAVE_ASSIGN_NODE_ID 2050        // v. >= 12
#define SUPLA_CALCFG_CMD_ZWAVE_GET_WAKE_UP_SETTINGS 2060  // v. >= 12
#define SUPLA_CALCFG_CMD_ZWAVE_SET_WAKE_UP_TIME 2070      // v. >= 12
#define SUPLA_CALCFG_CMD_ZWAVE_CONFIG_MODE_ACTIVE 4000    // v. >= 12
#define SUPLA_CALCFG_CMD_DEBUG_STRING 5000                // v. >= 12
#define SUPLA_CALCFG_CMD_PROGRESS_REPORT 5001             // v. >= 12
#define SUPLA_CALCFG_CMD_SET_LIGHTSOURCE_LIFESPAN 6000    // v. >= 12
#define SUPLA_CALCFG_CMD_RESET_COUNTERS 7000              // v. >= 15
#define SUPLA_CALCFG_CMD_RECALIBRATE 8000                 // v. >= 15
#define SUPLA_CALCFG_CMD_ENTER_CFG_MODE 9000              // v. >= 17
#define SUPLA_CALCFG_CMD_SET_TIME 9100                    // v. >= 21
#define SUPLA_CALCFG_CMD_START_SUBDEVICE_PAIRING 9200     // v. >= 25
#define SUPLA_CALCFG_CMD_IDENTIFY_DEVICE 9300             // v. >= 25
#define SUPLA_CALCFG_CMD_IDENTIFY_SUBDEVICE 9310          // v. >= 25
#define SUPLA_CALCFG_CMD_RESTART_DEVICE 9400              // v. >= 25
#define SUPLA_CALCFG_CMD_RESTART_SUBDEVICE 9410           // v. >= 25
#define SUPLA_CALCFG_CMD_TAKE_OCR_PHOTO 9420              // v. >= 25

#define SUPLA_CALCFG_DATATYPE_RS_SETTINGS 1000
#define SUPLA_CALCFG_DATATYPE_FB_SETTINGS 1100  // v. >= 17

#define CALCFG_ZWAVE_SCREENTYPE_UNKNOWN 0
#define CALCFG_ZWAVE_SCREENTYPE_MULTILEVEL 1
#define CALCFG_ZWAVE_SCREENTYPE_BINARY 2
#define CALCFG_ZWAVE_SCREENTYPE_MULTILEVEL_AUTOSHADE 3
#define CALCFG_ZWAVE_SCREENTYPE_MULTILEVEL_COLOR_CONTROL 4
#define CALCFG_ZWAVE_SCREENTYPE_BINARY_COLOR_CONTROL 5
#define CALCFG_ZWAVE_SCREENTYPE_SENSOR 6

#define ZWAVE_NODE_NAME_MAXSIZE 50

#define ZWAVE_NODE_FLAG_CHANNEL_ASSIGNED 0x1
#define ZWAVE_NODE_FLAG_WAKEUP_TIME_SETTABLE 0x2

typedef struct {
  unsigned char Id;
  unsigned char Flags;
  union {
    unsigned char ChannelNumber;
    _supla_int_t ChannelID;
  };
  unsigned char ScreenType;
  unsigned char NameSize;  // including the terminating null byte ('\0')
  char Name[ZWAVE_NODE_NAME_MAXSIZE];  // UTF8. Last variable in struct!
} TCalCfg_ZWave_Node;                  // v. >= 12

typedef struct {
  unsigned _supla_int_t MinimumSec : 24;
  unsigned _supla_int_t MaximumSec : 24;
  unsigned _supla_int_t ValueSec : 24;
  unsigned _supla_int_t IntervalStepSec : 24;
} TCalCfg_ZWave_WakeupSettingsReport;

typedef struct {
  unsigned _supla_int_t TimeSec : 24;
} TCalCfg_ZWave_WakeUpTime;

typedef struct {
  _supla_int_t Command;
  unsigned char Progress;  // 0 - 100%
} TCalCfg_ProgressReport;

typedef struct {
  unsigned char ResetCounter;                   // 0 - NO, 1 - YES
  unsigned char SetTime;                        // 0 - NO, 1 - YES
  unsigned _supla_int16_t LightSourceLifespan;  // 0 - 65535 hours
} TCalCfg_LightSourceLifespan;

#define SUPLA_CALCFG_SUBDEVICE_NAME_MAXSIZE 120

// Subdevice Pairing result is send in TDS_DeviceCalCfgResult. Possible values:
// SUPLA_CALCFG_RESULT_TRUE - paring result/status is in Data
// SUPLA_CALCFG_RESULT_UNAUTHORIZED - unauthorized
// SUPLA_CALCFG_RESULT_NOT_SUPPORTED - not supported
// Only in case of TRUE, TCalCfg_SubdevicePairingResult is included in
// TDS_DeviceCalCfgResult.
#define SUPLA_CALCFG_PAIRINGRESULT_PROCEDURE_STARTED 0
#define SUPLA_CALCFG_PAIRINGRESULT_ONGOING 1
#define SUPLA_CALCFG_PAIRINGRESULT_NO_NEW_DEVICE_FOUND 2
#define SUPLA_CALCFG_PAIRINGRESULT_SUCCESS 3
#define SUPLA_CALCFG_PAIRINGRESULT_DEVICE_NOT_SUPPORTED 4
#define SUPLA_CALCFG_PAIRINGRESULT_RESOURCES_LIMIT_EXCEEDED 5
#define SUPLA_CALCFG_PAIRINGRESULT_NOT_STARTED_NOT_READY 6
#define SUPLA_CALCFG_PAIRINGRESULT_NOT_STARTED_BUSY 7

typedef struct {
  unsigned _supla_int16_t
      ElapsedTimeSec;  // Time in seconds since procedure was started
  unsigned _supla_int16_t MaximumDurationSec;  // Time in seconds
  unsigned char PairingResult;                 // SUPLA_CALCFG_PAIRINGRESULT_
  unsigned char NameSize;  // including the terminating null byte ('\0')
  char Name[SUPLA_CALCFG_SUBDEVICE_NAME_MAXSIZE];  // UTF8. Last variable in
                                                   // struct!
} TCalCfg_SubdevicePairingResult;                  // v. >= 25

// CALCFG == CALIBRATION / CONFIG
typedef struct {
  _supla_int_t ChannelID;
  _supla_int_t Command;
  _supla_int_t DataType;
  unsigned _supla_int_t DataSize;
  char Data[SUPLA_CALCFG_DATA_MAXSIZE];  // Last variable in struct!
} TCS_DeviceCalCfgRequest;               // v. >= 10

// CALCFG == CALIBRATION / CONFIG
typedef struct {
  _supla_int_t Id;
  char Target;  // SUPLA_TARGET_
  _supla_int_t Command;
  _supla_int_t DataType;
  unsigned _supla_int_t DataSize;
  char Data[SUPLA_CALCFG_DATA_MAXSIZE];  // Last variable in struct!
} TCS_DeviceCalCfgRequest_B;             // v. >= 11

typedef struct {
  _supla_int_t ChannelID;
  _supla_int_t Command;
  _supla_int_t Result;
  unsigned _supla_int_t DataSize;
  char Data[SUPLA_CALCFG_DATA_MAXSIZE];  // Last variable in struct!
} TSC_DeviceCalCfgResult;                // v. >= 10

typedef struct {
  _supla_int_t SenderID;
  _supla_int_t ChannelNumber;
  _supla_int_t Command;
  char SuperUserAuthorized;
  _supla_int_t DataType;
  unsigned _supla_int_t DataSize;
  char Data[SUPLA_CALCFG_DATA_MAXSIZE];  // Last variable in struct!
} TSD_DeviceCalCfgRequest;               // v. >= 10

typedef struct {
  _supla_int_t ReceiverID;
  _supla_int_t ChannelNumber;
  _supla_int_t Command;
  _supla_int_t Result;
  unsigned _supla_int_t DataSize;
  char Data[SUPLA_CALCFG_DATA_MAXSIZE];  // Last variable in struct!
} TDS_DeviceCalCfgResult;                // v. >= 10

// Used in CALCFG recalibrate command
typedef struct {
  _supla_int_t FullOpeningTimeMS;
  _supla_int_t FullClosingTimeMS;
} TCalCfg_RollerShutterSettings;

#define RGBW_BRIGHTNESS_ONOFF 0x1
#define RGBW_COLOR_ONOFF 0x2

// Values from other fields are applied in a standard way
#define RGBW_COMMAND_NOT_SET 0
// Ignores all other bytes and turns on the dimmer
#define RGBW_COMMAND_TURN_ON_DIMMER 1
// Ignores all other bytes and turns off the dimmer
#define RGBW_COMMAND_TURN_OFF_DIMMER 2
// Ignores all other bytes and toggles the dimmer
#define RGBW_COMMAND_TOGGLE_DIMMER 3
// Ignores all other bytes and turns on the RGB
#define RGBW_COMMAND_TURN_ON_RGB 4
// Ignores all other bytes and turns off the RGB
#define RGBW_COMMAND_TURN_OFF_RGB 5
// Ignores all other bytes and toggles the RGB
#define RGBW_COMMAND_TOGGLE_RGB 6
// Ignores all other bytes and turns on the RGB and Dimmer
#define RGBW_COMMAND_TURN_ON_ALL 7
// Ignores all other bytes and turns off the RGB and Dimmer
#define RGBW_COMMAND_TURN_OFF_ALL 8
// Ignores all other bytes and toggles the RGB and Dimmer (with sync, so both
// will be off or both will be on)
#define RGBW_COMMAND_TOGGLE_ALL 9
// Stores brightness value and ignores all other bytes, if dimmer is off, it
// stays off
#define RGBW_COMMAND_SET_BRIGHTNESS_WITHOUT_TURN_ON 10
// Stores colorBrightness value and ignores all other bytes.
// If RGB is off, it stays off
#define RGBW_COMMAND_SET_COLOR_BRIGHTNESS_WITHOUT_TURN_ON 11
// Stores color value (R, G, B) and ignores all other bytes.
// If RGB is off, it stays off
#define RGBW_COMMAND_SET_RGB_WITHOUT_TURN_ON 12
// Start brightness dimmer iteration
#define RGBW_COMMAND_START_ITERATE_DIMMER 13
// Start color brightness iteration
#define RGBW_COMMAND_START_ITERATE_RGB 14
// Start dimmer and rgb brightness iteration
#define RGBW_COMMAND_START_ITERATE_ALL 15
// Stop brightness dimmer iteration
#define RGBW_COMMAND_STOP_ITERATE_DIMMER 16
// Stop color brightness iteration
#define RGBW_COMMAND_STOP_ITERATE_RGB 17
// Stop dimmer and rgb brightness iteration
#define RGBW_COMMAND_STOP_ITERATE_ALL 18

typedef struct {
  char brightness;
  char colorBrightness;
  char B;
  char G;
  char R;
  char onOff;
  char command;  // RGBW_COMMAND_, requires
                 // SUPLA_CHANNEL_FLAG_RGBW_COMMANDS_SUPPORTED v. >= 21
} TRGBW_Value;   // v. >= 10

#define SUPLA_RELAY_FLAG_OVERCURRENT_RELAY_OFF 0x1

typedef struct {
  char hi;  // actual state of relay  - 0 turned off, >= 1 - turned on
  unsigned _supla_int16_t flags;  // SUPLA_RELAY_FLAG_*
} TRelayChannel_Value;            // v. >= 15

#define DIGIGLASS_TOO_LONG_OPERATION_WARNING 0x1
#define DIGIGLASS_PLANNED_REGENERATION_IN_PROGRESS 0x2
#define DIGIGLASS_REGENERATION_AFTER_20H_IN_PROGRESS 0x4

typedef struct {
  unsigned char flags;
  unsigned char sectionCount;    // 1 - 16 Filled by server
  unsigned _supla_int16_t mask;  // bit mask. 0 - opaque, 1 - transparent
} TDigiglass_Value;              // v. >= 14

typedef struct {
  unsigned _supla_int16_t mask;  // Bit mask. 0 - opaque, 1 - transparent
  unsigned _supla_int16_t
      active_bits;          // Specifies which bits of the mask are not skipped
} TCSD_Digiglass_NewValue;  // v. >= 14

typedef struct {
  unsigned char sec;        // 0-59
  unsigned char min;        // 0-59
  unsigned char hour;       // 0-24
  unsigned char dayOfWeek;  // 1 = Sunday, 2 = Monday, …, 7 = Saturday
} TThermostat_Time;         // v. >= 11

#define THERMOSTAT_SCHEDULE_DAY_SUNDAY 0x01
#define THERMOSTAT_SCHEDULE_DAY_MONDAY 0x02
#define THERMOSTAT_SCHEDULE_DAY_TUESDAY 0x04
#define THERMOSTAT_SCHEDULE_DAY_WEDNESDAY 0x08
#define THERMOSTAT_SCHEDULE_DAY_THURSDAY 0x10
#define THERMOSTAT_SCHEDULE_DAY_FRIDAY 0x20
#define THERMOSTAT_SCHEDULE_DAY_SATURDAY 0x40
#define THERMOSTAT_SCHEDULE_DAY_ALL 0xFF

#define THERMOSTAT_SCHEDULE_HOURVALUE_TYPE_TEMPERATURE 0
#define THERMOSTAT_SCHEDULE_HOURVALUE_TYPE_PROGRAM 1

typedef struct {
  unsigned char ValueType;  // THERMOSTAT_SCHEDULE_HOURVALUE_TYPE_
  char HourValue[7][24];    // 7 days x 24h
                            // 0 = Sunday, 1 = Monday, …, 6 = Saturday
} TThermostat_Schedule;     // v. >= 11

typedef struct {
  unsigned char ValueType;  // THERMOSTAT_SCHEDULE_HOURVALUE_TYPE_
  unsigned char WeekDays;   // THERMOSTAT_SCHEDULE_DAY_
  char HourValue[24];
} TThermostatValueGroup;  // v. >= 11

// Used in Heatpol thermostat
typedef struct {
  TThermostatValueGroup Group[4];
} TThermostat_ScheduleCfg;  // v. >= 11

// Temperature definitions for Heatpol thermostat
// TThermostatTemperatureCfg
#define TEMPERATURE_INDEX1 0x0001
#define TEMPERATURE_INDEX2 0x0002
#define TEMPERATURE_INDEX3 0x0004
#define TEMPERATURE_INDEX4 0x0008
#define TEMPERATURE_INDEX5 0x0010
#define TEMPERATURE_INDEX6 0x0020
#define TEMPERATURE_INDEX7 0x0040
#define TEMPERATURE_INDEX8 0x0080
#define TEMPERATURE_INDEX9 0x0100
#define TEMPERATURE_INDEX10 0x0200
// TThermostatTemperatureCfg has only 10 fields

// Used in Heatpol thermostat
typedef struct {
  _supla_int16_t Index;  // BIT0 Temperature[0], BIT1 Temperature[1] etc...
  unsigned _supla_int16_t Temperature[10];
} TThermostatTemperatureCfg;

// Temperature definitions for HVAC
// THVACTemperatureCfg
// Below values are settable by user in UI
// Temperature below which heating will be enabled as a freeze protection
#define TEMPERATURE_FREEZE_PROTECTION (1ULL << 0)
// Economic temperaturey
#define TEMPERATURE_ECO (1ULL << 1)
// Comfort temperature
#define TEMPERATURE_COMFORT (1ULL << 2)
#define TEMPERATURE_BOOST (1ULL << 3)
// Temperature above which cooling will be enabled as an overheating protection.
#define TEMPERATURE_HEAT_PROTECTION (1ULL << 4)
// Histeresis value - i.e. heating will be enabled when current temperature
// is histeresis/2 lower than current setpoint.
#define TEMPERATURE_HISTERESIS (1ULL << 5)
// AUX histeresis value - used to determine heating based on AUX temperature
// If aux histeresis is missing, then TEMPERATURE_HISTERESIS is used
#define TEMPERATURE_AUX_HISTERESIS (1ULL << 18)
// Turns on "alarm" when temperature is below this value. Can be visual effect
// or sound (if device is capable). It can also send AT to server (TBD)
#define TEMPERATURE_BELOW_ALARM (1ULL << 6)
// As above, but for tempertatures above threshold
#define TEMPERATURE_ABOVE_ALARM (1ULL << 7)
// Minimum temperature to be kept on heat/cold source (i.e. keeps floor always
// at least 19 degrees)
#define TEMPERATURE_AUX_MIN_SETPOINT (1ULL << 8)
// Maximum temperature to be kept on heat/cold source (i.e. protect floor
// from overheating and keep at most 28 degrees)
#define TEMPERATURE_AUX_MAX_SETPOINT (1ULL << 9)

// Below values are readonly for user
// Minimum room (main thermometer) temperature to be set by user
#define TEMPERATURE_ROOM_MIN (1ULL << 10)
// Maximum room (main thermometer) temperature to be set by user
#define TEMPERATURE_ROOM_MAX (1ULL << 11)
// Minimum allowed temperature for heat/cold source (used with second
// thermomter for floor/water/etc.)
#define TEMPERATURE_AUX_MIN (1ULL << 12)
// Maximum allowed temperature for heat/cold source
#define TEMPERATURE_AUX_MAX (1ULL << 13)
// Minimum histereis value
#define TEMPERATURE_HISTERESIS_MIN (1ULL << 14)
// Maximum histereis value
#define TEMPERATURE_HISTERESIS_MAX (1ULL << 15)
// Minimum temperature offset in HEAT_COOL mode
#define TEMPERATURE_HEAT_COOL_OFFSET_MIN (1ULL << 16)
// Maximum temperature offset in HEAT_COOL mode
#define TEMPERATURE_HEAT_COOL_OFFSET_MAX (1ULL << 17)
// 5 values left for future use (value << 18 is defined earlier)

#define SUPLA_TEMPERATURE_INVALID_INT16 -32768

// Used in Supla Thermostat
typedef struct {
  unsigned _supla_int_t Index;  // BIT0 Temperature[0], BIT1 Temperature[1]
                                // etc...
  _supla_int16_t Temperature[24];
} THVACTemperatureCfg;

// Heatpol: Thermostat configuration commands - ver. >= 11
#define SUPLA_THERMOSTAT_CMD_TURNON 1
#define SUPLA_THERMOSTAT_CMD_SET_MODE_AUTO 2
#define SUPLA_THERMOSTAT_CMD_SET_MODE_COOL 3
#define SUPLA_THERMOSTAT_CMD_SET_MODE_HEAT 4
#define SUPLA_THERMOSTAT_CMD_SET_MODE_NORMAL 5
#define SUPLA_THERMOSTAT_CMD_SET_MODE_ECO 6
#define SUPLA_THERMOSTAT_CMD_SET_MODE_TURBO 7
#define SUPLA_THERMOSTAT_CMD_SET_MODE_DRY 8
#define SUPLA_THERMOSTAT_CMD_SET_MODE_FANONLY 9
#define SUPLA_THERMOSTAT_CMD_SET_MODE_PURIFIER 10
#define SUPLA_THERMOSTAT_CMD_SET_SCHEDULE 11
#define SUPLA_THERMOSTAT_CMD_SET_TIME 12
#define SUPLA_THERMOSTAT_CMD_SET_TEMPERATURE 13

// Heatpol: Thermostat value flags - ver. >= 11
#define SUPLA_THERMOSTAT_VALUE_FLAG_ON 0x0001
#define SUPLA_THERMOSTAT_VALUE_FLAG_AUTO_MODE 0x0002
#define SUPLA_THERMOSTAT_VALUE_FLAG_COOL_MODE 0x0004
#define SUPLA_THERMOSTAT_VALUE_FLAG_HEAT_MODE 0x0008
#define SUPLA_THERMOSTAT_VALUE_FLAG_ECO_MODE 0x0010
#define SUPLA_THERMOSTAT_VALUE_FLAG_DRY_MODE 0x0020
#define SUPLA_THERMOSTAT_VALUE_FLAG_FANONLY_MODE 0x0040
#define SUPLA_THERMOSTAT_VALUE_FLAG_PURIFIER_MODE 0x0080

// Heatpol: Thermostat fields - ver. >= 11
#define THERMOSTAT_FIELD_MeasuredTemperatures 0x01
#define THERMOSTAT_FIELD_PresetTemperatures 0x02
#define THERMOSTAT_FIELD_Flags 0x04
#define THERMOSTAT_FIELD_Values 0x08
#define THERMOSTAT_FIELD_Time 0x10
#define THERMOSTAT_FIELD_Schedule 0x20

// Used in Heatpol only
typedef struct {
  unsigned char Fields;
  _supla_int16_t MeasuredTemperature[10];  // * 0.01
  _supla_int16_t PresetTemperature[10];    // * 0.01
  _supla_int16_t Flags[8];
  _supla_int16_t Values[8];
  TThermostat_Time Time;
  TThermostat_Schedule Schedule;  // 7 days x 24h (4bit/hour)
} TThermostat_ExtendedValue;      // v. >= 11

// Used in Heatpol only
typedef struct {
  unsigned char IsOn;
  unsigned char Flags;
  _supla_int16_t MeasuredTemperature;  // * 0.01
  _supla_int16_t PresetTemperature;    // * 0.01
} TThermostat_Value;                   // v. >= 11

typedef struct {
  unsigned _supla_int16_t year;
  unsigned char month;
  unsigned char day;
  unsigned char dayOfWeek;  // 1 = Sunday, 2 = Monday, …, 7 = Saturday
  unsigned char hour;
  unsigned char min;
  unsigned char sec;
  unsigned _supla_int_t
      timezoneSize;  // including the terminating null byte ('\0')
  char timezone[SUPLA_TIMEZONE_MAXSIZE];  // Last variable in struct!
} TSDC_UserLocalTimeResult;

typedef struct {
  _supla_int_t SenderID;
  union {
    _supla_int_t ChannelID;       // Client -> Server
    unsigned char ChannelNumber;  // Server -> Device
  };
} TCSD_ChannelStateRequest;  // v. >= 12 Client -> Server -> Device

#define SUPLA_CHANNELSTATE_FIELD_IPV4 0x0001
#define SUPLA_CHANNELSTATE_FIELD_MAC 0x0002
#define SUPLA_CHANNELSTATE_FIELD_BATTERYLEVEL 0x0004
#define SUPLA_CHANNELSTATE_FIELD_BATTERYPOWERED 0x0008
#define SUPLA_CHANNELSTATE_FIELD_WIFIRSSI 0x0010
#define SUPLA_CHANNELSTATE_FIELD_WIFISIGNALSTRENGTH 0x0020
#define SUPLA_CHANNELSTATE_FIELD_BRIDGENODESIGNALSTRENGTH 0x0040
#define SUPLA_CHANNELSTATE_FIELD_UPTIME 0x0080
#define SUPLA_CHANNELSTATE_FIELD_CONNECTIONUPTIME 0x0100
#define SUPLA_CHANNELSTATE_FIELD_BATTERYHEALTH 0x0200
#define SUPLA_CHANNELSTATE_FIELD_BRIDGENODEONLINE 0x0400
#define SUPLA_CHANNELSTATE_FIELD_LASTCONNECTIONRESETCAUSE 0x0800
// LIGHTSOURCELIFESPAN, LIGHTSOURCEOPERATINGTIME, and OPERATINGTIME are
// mutually exclusive. Use only one of them.
#define SUPLA_CHANNELSTATE_FIELD_LIGHTSOURCELIFESPAN 0x1000
#define SUPLA_CHANNELSTATE_FIELD_LIGHTSOURCEOPERATINGTIME 0x2000
#define SUPLA_CHANNELSTATE_FIELD_OPERATINGTIME 0x4000
// SWITCHCYCLECOUNT and defualtIconField are is mutually exclusive. Use only one
// of them.
#define SUPLA_CHANNELSTATE_FIELD_SWITCHCYCLECOUNT 0x8000

#define SUPLA_LASTCONNECTIONRESETCAUSE_UNKNOWN 0
#define SUPLA_LASTCONNECTIONRESETCAUSE_ACTIVITY_TIMEOUT 1
#define SUPLA_LASTCONNECTIONRESETCAUSE_WIFI_CONNECTION_LOST 2
#define SUPLA_LASTCONNECTIONRESETCAUSE_SERVER_CONNECTION_LOST 3

typedef struct {
  _supla_int_t ReceiverID;  // Not used in extended values
  union {
    // Not used in extended values
    _supla_int_t ChannelID;       // Server -> Client
    unsigned char ChannelNumber;  // Device -> Server
  };
  _supla_int_t Fields;  // SUPLA_CHANNELSTATE_FIELD_*
  union {
    _supla_int_t defaultIconField;
    unsigned _supla_int_t SwitchCycleCount;
  };
  unsigned _supla_int_t IPv4;
  unsigned char MAC[6];
  unsigned char BatteryLevel;    // 0 - 100%
  unsigned char BatteryPowered;  // true(1)/false(0)
  char WiFiRSSI;
  unsigned char WiFiSignalStrength;        // 0 - 100%
  unsigned char BridgeNodeOnline;          // 0/1
  unsigned char BridgeNodeSignalStrength;  // 0 - 100%
  unsigned _supla_int_t Uptime;            // sec.
  unsigned _supla_int_t ConnectionUptime;  // sec.
  unsigned char BatteryHealth;
  unsigned char LastConnectionResetCause;  // SUPLA_LASTCONNECTIONRESETCAUSE_*
  unsigned _supla_int16_t LightSourceLifespan;  // 0 - 65535 hours
  union {
    _supla_int16_t LightSourceLifespanLeft;  // -327,67 - 100.00%
                                             // LightSourceLifespan * 0.01
    _supla_int_t LightSourceOperatingTime;   // -3932100sec. - 3932100sec.
    unsigned _supla_int_t OperatingTime;     // time in seconds
  };
  char EmptySpace[2];  // Empty space for future use
} TDSC_ChannelState;   // v. >= 12 Device -> Server -> Client

#define TChannelState_ExtendedValue TDSC_ChannelState

typedef struct {
  _supla_int_t ChannelID;
} TCS_ChannelBasicCfgRequest;  // v. >= 12

typedef struct {
  union {
    // Remaining time to turn off
    unsigned _supla_int_t RemainingTimeMs;
    unsigned _supla_int_t RemainingTimeS;
    unsigned _supla_int_t CountdownEndsAt;  // Unix timestamp - Filled by server
  };

  unsigned char TargetValue[SUPLA_CHANNELVALUE_SIZE];

  _supla_int_t SenderID;
  unsigned _supla_int_t
      SenderNameSize;  // including the terminating null byte ('\0')
  char SenderName[SUPLA_SENDER_NAME_MAXSIZE];  // Last variable in struct!
                                               // UTF8 | Filled by server
} TTimerState_ExtendedValue;

typedef struct {
  TChannelState_ExtendedValue Channel;
  TTimerState_ExtendedValue Timer;  // Last variable in struct!
} TChannelAndTimerState_ExtendedValue;

typedef struct {
  char DeviceName[SUPLA_DEVICE_NAME_MAXSIZE];  // UTF8
  char DeviceSoftVer[SUPLA_SOFTVER_MAXSIZE];
  _supla_int_t DeviceID;
  _supla_int_t DeviceFlags;
  _supla_int16_t ManufacturerID;
  _supla_int16_t ProductID;

  _supla_int_t ID;
  unsigned char Number;
  _supla_int_t Type;
  _supla_int_t Func;
  _supla_int_t FuncList;

  unsigned _supla_int_t ChannelFlags;
  unsigned _supla_int_t
      CaptionSize;  // including the terminating null byte ('\0')
  char Caption[SUPLA_CHANNEL_CAPTION_MAXSIZE];  // Last variable in struct!
} TSC_ChannelBasicCfg;                          // v. >= 12

typedef struct {
  _supla_int_t ChannelID;
  _supla_int_t Func;
} TCS_SetChannelFunction;  // v. >= 12

typedef struct {
  _supla_int_t ChannelID;
  _supla_int_t Func;
  unsigned char ResultCode;
} TSC_SetChannelFunctionResult;  // v. >= 12

typedef struct {
  union {
    unsigned char ChannelNumber;
    _supla_int_t ID;
  };

  unsigned _supla_int_t
      CaptionSize;  // including the terminating null byte ('\0')
  char Caption[SUPLA_CAPTION_MAXSIZE];  // Last variable in struct!
} TDCS_SetCaption;                      // v. >= 12

typedef struct {
  union {
    unsigned char ChannelNumber;
    _supla_int_t ID;
  };

  unsigned char ResultCode;
  unsigned _supla_int_t
      CaptionSize;  // including the terminating null byte ('\0')
  char Caption[SUPLA_CAPTION_MAXSIZE];  // Last variable in struct!
} TSCD_SetCaptionResult;                // v. >= 12

typedef struct {
  unsigned char ResultCode;
} TSC_ClientsReconnectRequestResult;  // v. >= 12

typedef struct {
  // Disabled: 0
  // Ignore: <0
  _supla_int_t IODeviceRegistrationTimeSec;
  _supla_int_t ClientRegistrationTimeSec;
} TCS_SetRegistrationEnabled;  // v. >= 12

typedef struct {
  unsigned char ResultCode;
} TSC_SetRegistrationEnabledResult;  // v. >= 12

typedef struct {
  _supla_int_t DeviceID;
} TCS_DeviceReconnectRequest;  // v. >= 12

typedef struct {
  _supla_int_t DeviceID;
  unsigned char ResultCode;
} TSC_DeviceReconnectRequestResult;  // v. >= 12

typedef struct {
  // server -> device

  unsigned char ChannelCount;
  _supla_int_t Functions[SUPLA_CHANNELMAXCOUNT];  // Last variable in struct!
  // Functions[ChannelNumber]
} TSD_ChannelFunctions;  // ver. >= 12

typedef struct {
  unsigned char ChannelNumber;
} TDS_GetChannelIntParamsRequest;

typedef struct {
  unsigned char ChannelNumber;
  _supla_int_t Param1;
  _supla_int_t Param2;
  _supla_int_t Param3;
} TSD_ChannelIntParams;

#define SUPLA_CHANNEL_CONFIG_MAXSIZE 512  // v. <= 19 - 128; v. >= 21 - 512
#define SUPLA_DEVICE_CONFIG_MAXSIZE 512

// Default type is used for standard channel config (user modifiable)
#define SUPLA_CONFIG_TYPE_DEFAULT 0
// Weekly schedule
#define SUPLA_CONFIG_TYPE_WEEKLY_SCHEDULE 2
// For SUPLA_CHANNELFNC_HVAC_THERMOSTAT, ALT weekly schedule is used for
// cooling subfuction, while standard weelkly schedule is used for heating
#define SUPLA_CONFIG_TYPE_ALT_WEEKLY_SCHEDULE 3
#define SUPLA_CONFIG_TYPE_OCR 4

/********************************************
 * DEVICE CONFIG STRUCTURES
 ********************************************/

// SUPLA_DS_CALL_SET_DEVICE_CONFIG
// SUPLA_SD_CALL_SET_DEVICE_CONFIG
// Config field should contain fields stored in order as they appear in
// Fields parameter. Size of parameter depends on Field type.
typedef struct {
  unsigned char EndOfDataFlag;  // 1 - last message; 0 - more messages will come
  unsigned char zero[8];        // for future use
  unsigned _supla_int64_t
      AvailableFields;             // bit map of SUPLA_DEVICE_CONFIG_FIELD_
  unsigned _supla_int64_t Fields;  // bit map of SUPLA_DEVICE_CONFIG_FIELD_
  unsigned _supla_int16_t ConfigSize;
  char Config[SUPLA_DEVICE_CONFIG_MAXSIZE];  // Last variable in struct!
} TSDS_SetDeviceConfig;                      // v. >= 21

// SUPLA_SD_CALL_SET_DEVICE_CONFIG_RESULT
// SUPLA_DS_CALL_SET_DEVICE_CONFIG_RESULT
typedef struct {
  unsigned char Result;   // SUPLA_CONFIG_RESULT_*
  unsigned char zero[9];  // for future use
} TSDS_SetDeviceConfigResult;

typedef struct {
  _supla_int_t DeviceId;
  unsigned char EndOfDataFlag;  // 1 - last message; 0 - more messages will come
  unsigned char zero[8];        // for future use
  unsigned _supla_int64_t
      AvailableFields;             // bit map of SUPLA_DEVICE_CONFIG_FIELD_
  unsigned _supla_int64_t Fields;  // bit map of SUPLA_DEVICE_CONFIG_FIELD_
  unsigned _supla_int16_t ConfigSize;
  char Config[SUPLA_DEVICE_CONFIG_MAXSIZE];  // Last variable in struct!
} TSCS_DeviceConfig;                         // v. >= 21

// SUPLA_SC_CALL_DEVICE_CONFIG_UPDATE_OR_RESULT
typedef struct {
  unsigned char Result;      // SUPLA_CONFIG_RESULT_*. It matters when it is a
                             // response to SUPLA_CS_CALL_GET_DEVICE_CONFIG
  TSCS_DeviceConfig Config;  // Last variable in struct!
} TSC_DeviceConfigUpdateOrResult;

// SUPLA_CS_CALL_GET_DEVICE_CONFIG
typedef struct {
  _supla_int_t DeviceId;
  unsigned _supla_int64_t Fields;  // bit map of SUPLA_DEVICE_CONFIG_FIELD_
  unsigned char zero[8];           // for future use
} TCS_GetDeviceConfigRequest;

#define SUPLA_DEVCFG_STATUS_LED_ON_WHEN_CONNECTED 0
#define SUPLA_DEVCFG_STATUS_LED_OFF_WHEN_CONNECTED 1
#define SUPLA_DEVCFG_STATUS_LED_ALWAYS_OFF 2

typedef struct {
  unsigned char StatusLedType;  // SUPLA_DEVCFG_STATUS_LED_
} TDeviceConfig_StatusLed;      // v. >= 21

typedef struct {
  unsigned char Disabled;        // 1 - true; 0 - false
} TDeviceConfig_PowerStatusLed;  // v. >= 25

typedef struct {
  unsigned char ScreenBrightness;  // 0-100%
  unsigned char Automatic;         // 0 - false; 1 - true
  signed char AdjustmentForAutomatic;
} TDeviceConfig_ScreenBrightness;  // v. >= 21

typedef struct {
  unsigned char Volume;        // 0-100%
} TDeviceConfig_ButtonVolume;  // v. >= 21

typedef struct {
  unsigned char DisableUserInterface;  // 0 - false (local UI enabled)
                                       // 1 - true (local UI disabled)
                                       // 2 - partial
  // min/max allowed parameters are mandatory for "partial" variant
  unsigned _supla_int16_t minAllowedTemperatureSetpointFromLocalUI;
  unsigned _supla_int16_t maxAllowedTemperatureSetpointFromLocalUI;
} TDeviceConfig_DisableUserInterface;  // v. >= 21

typedef struct {
  unsigned char AutomaticTimeSync;  // 0 - disabled
                                    // 1 - enabled
} TDeviceConfig_AutomaticTimeSync;  // v. >= 21

typedef struct {
  unsigned _supla_int16_t HomeScreenOffDelayS;  // delay in seconds
                                                // 0 - disabled
} TDeviceConfig_HomeScreenOffDelay;             // v. >= 21

#define SUPLA_DEVCFG_HOME_SCREEN_OFF_DELAY_TYPE_ALWAYS_ENABLED 0
#define SUPLA_DEVCFG_HOME_SCREEN_OFF_DELAY_TYPE_ENABLED_WHEN_DARK 1
typedef struct {
  unsigned char
      HomeScreenOffDelayType;  // SUPLA_DEVCFG_HOME_SCREEN_OFF_DELAY_TYPE_
} TDeviceConfig_HomeScreenOffDelayType;  // v. >= 24

#define SUPLA_DEVCFG_HOME_SCREEN_CONTENT_NONE (1ULL << 0)
#define SUPLA_DEVCFG_HOME_SCREEN_CONTENT_TEMPERATURE (1ULL << 1)
#define SUPLA_DEVCFG_HOME_SCREEN_CONTENT_TEMPERATURE_AND_HUMIDITY (1ULL << 2)
#define SUPLA_DEVCFG_HOME_SCREEN_CONTENT_TIME (1ULL << 3)
#define SUPLA_DEVCFG_HOME_SCREEN_CONTENT_TIME_DATE (1ULL << 4)
#define SUPLA_DEVCFG_HOME_SCREEN_CONTENT_TEMPERATURE_TIME (1ULL << 5)
#define SUPLA_DEVCFG_HOME_SCREEN_CONTENT_MAIN_AND_AUX_TEMPERATURE (1ULL << 6)
#define SUPLA_DEVCFG_HOME_SCREEN_CONTENT_MODE_OR_TEMPERATURE (1ULL << 7)

typedef struct {
  // bit field with all available modes (reported by device, readonly for other
  // components)
  unsigned _supla_int64_t ContentAvailable;
  // configured mode (settable)
  unsigned _supla_int64_t
      HomeScreenContent;            // SUPLA_DEVCFG_HOME_SCREEN_CONTENT_
} TDeviceConfig_HomeScreenContent;  // v. >= 21

/********************************************
 * CHANNEL CONFIG STRUCTURES
 ********************************************/

#define SUPLA_CONFIG_RESULT_FALSE 0
#define SUPLA_CONFIG_RESULT_TRUE 1
#define SUPLA_CONFIG_RESULT_DATA_ERROR 2
#define SUPLA_CONFIG_RESULT_TYPE_NOT_SUPPORTED 3
#define SUPLA_CONFIG_RESULT_FUNCTION_NOT_SUPPORTED 4
#define SUPLA_CONFIG_RESULT_LOCAL_CONFIG_DISABLED 5
#define SUPLA_CONFIG_RESULT_NOT_ALLOWED 6
#define SUPLA_CONFIG_RESULT_DEVICE_NOT_FOUND 7

// SUPLA_CS_CALL_GET_CHANNEL_CONFIG
typedef struct {
  _supla_int_t ChannelId;
  unsigned char ConfigType;  // SUPLA_CONFIG_TYPE_
  unsigned _supla_int_t Flags;
} TCS_GetChannelConfigRequest;  // v. >= 21

// SUPLA_CS_CALL_SET_CHANNEL_CONFIG
typedef struct {
  _supla_int_t ChannelId;
  _supla_int_t Func;
  unsigned char ConfigType;  // SUPLA_CONFIG_TYPE_
  unsigned _supla_int16_t ConfigSize;
  char Config[SUPLA_CHANNEL_CONFIG_MAXSIZE];  // Last variable in struct!
                                              // v. >= 21
                                              // TChannelConfig_*
} TSCS_ChannelConfig;

// SUPLA_SC_CALL_CHANNEL_CONFIG_UPDATE
typedef struct {
  unsigned char Result;       // SUPLA_CONFIG_RESULT_*. It matters when it is a
                              // response to SUPLA_CS_CALL_GET_CHANNEL_CONFIG or
                              // SUPLA_CS_CALL_SET_CHANNEL_CONFIG
  TSCS_ChannelConfig Config;  // Last variable in struct!
} TSC_ChannelConfigUpdateOrResult;

// SUPLA_DS_CALL_GET_CHANNEL_CONFIG
typedef struct {
  unsigned char ChannelNumber;
  unsigned char ConfigType;  // SUPLA_CONFIG_TYPE_
  unsigned _supla_int_t Flags;
} TDS_GetChannelConfigRequest;  // v. >= 16

// SUPLA_SD_CALL_GET_CHANNEL_CONFIG_RESULT
// SUPLA_DS_CALL_SET_CHANNEL_CONFIG
// SUPLA_SD_CALL_SET_CHANNEL_CONFIG
typedef struct {
  unsigned char ChannelNumber;
  _supla_int_t Func;
  unsigned char ConfigType;  // SUPLA_CONFIG_TYPE_
  unsigned _supla_int16_t ConfigSize;
  char Config[SUPLA_CHANNEL_CONFIG_MAXSIZE];  // Last variable in struct!
                                              // v. >= 16
                                              // TChannelConfig_*
} TSD_ChannelConfig;

// SUPLA_DS_CALL_SET_CHANNEL_CONFIG
// SUPLA_SD_CALL_SET_CHANNEL_CONFIG
typedef TSD_ChannelConfig TSDS_SetChannelConfig;  // v. >= 21

// SUPLA_SD_CALL_SET_CHANNEL_CONFIG_RESULT
// SUPLA_DS_CALL_SET_CHANNEL_CONFIG_RESULT
typedef struct {
  unsigned char Result;      // SUPLA_CONFIG_RESULT_*
  unsigned char ConfigType;  // SUPLA_CONFIG_TYPE_
  unsigned char ChannelNumber;
} TSDS_SetChannelConfigResult;

// SUPLA_SD_CALL_CHANNEL_CONFIG_FINISHED
typedef struct {
  unsigned char ChannelNumber;
} TSD_ChannelConfigFinished;

typedef struct {
  _supla_int_t TimeMS;
} TChannelConfig_StaircaseTimer;  // v. >= 16

typedef struct {
  _supla_int_t ClosingTimeMS;
  _supla_int_t OpeningTimeMS;
  unsigned char MotorUpsideDown;    // 0 - not set/not used, 1 - false, 2 - true
  unsigned char ButtonsUpsideDown;  // 0 - not set/not used, 1 - false, 2 - true
  signed char TimeMargin;  // -1 default (device specific), 0 - not set/not used
                           // 1 - no margin,
                           // > 1 - 51% of opening/closing time added on extreme
                           // positions - value should be decremented by 1.
  unsigned char VisualizationType;  // 0 - default, other values depends on
                                    // Cloud and App support
  unsigned char Reserved[32];
} TChannelConfig_RollerShutter;  // v. >= 16

typedef TChannelConfig_RollerShutter TChannelConfig_TerraceAwning;
typedef TChannelConfig_RollerShutter TChannelConfig_ProjectorScreen;
typedef TChannelConfig_RollerShutter TChannelConfig_Curtain;
typedef TChannelConfig_RollerShutter TChannelConfig_RollerGarageDoor;

typedef struct {
  _supla_int_t ClosingTimeMS;
  _supla_int_t OpeningTimeMS;
  _supla_int_t TiltingTimeMS;
  unsigned char MotorUpsideDown;    // 0 - not set/not used, 1 - false, 2 - true
  unsigned char ButtonsUpsideDown;  // 0 - not set/not used, 1 - false, 2 - true
  signed char TimeMargin;  // -1 default (device specific), 0 - not set/not used
                           // 1 - no margin,
                           // > 1 - 51% of opening/closing time added on extreme
                           // positions - value should be decremented by 1.
  unsigned _supla_int16_t
      Tilt0Angle;  // 0 - 180 - degree corresponding to tilt 0
  unsigned _supla_int16_t
      Tilt100Angle;               // 0 - 180 - degree corresponding to tilt 100
  unsigned char TiltControlType;  // SUPLA_TILT_CONTROL_TYPE_
  unsigned char VisualizationType;  // 0 - default, other values depends on
                                    // Cloud and App support
  unsigned char Reserved[32];
} TChannelConfig_FacadeBlind;  // v. >= 24

typedef TChannelConfig_FacadeBlind TChannelConfig_VerticalBlind;

typedef struct {
  unsigned _supla_int_t ActiveActions;
} TChannelConfig_ActionTrigger;  // v. >= 16

// Weekly schedule definition for HVAC channel
typedef struct {
  unsigned char Mode;  // for HVAC: SUPLA_HVAC_MODE_
  union {
    _supla_int16_t SetpointTemperatureHeat;  // * 0.01 - used for heating
    _supla_int16_t Value1;
  };
  union {
    _supla_int16_t SetpointTemperatureCool;  // * 0.01 - used for cooling
    _supla_int16_t Value2;
  };
} TWeeklyScheduleProgram;

#define SUPLA_WEEKLY_SCHEDULE_PROGRAMS_MAX_SIZE 4
#define SUPLA_WEEKLY_SCHEDULE_VALUES_SIZE (7 * 24 * 4)

typedef struct {
  // 4*5 = 20 B
  TWeeklyScheduleProgram Program[SUPLA_WEEKLY_SCHEDULE_PROGRAMS_MAX_SIZE];
  // "Quarters" contain Program setting for each 15 min. One 15 min program is
  // set on 4 bits, so in one byte we have settings for two 2x 15 min. 0 - off
  // 1 - program 1
  // 2 - program 2
  // 3 - program 3
  // 4 - program 4
  // (Quarter[0] & 0x0F)      is Sunday, 0:00 to 0:14
  // (Quarter[0] & 0xF0) >> 4 is Sunday, 0:15 to 0:29
  // Days of week are numbered: 0 - Sunday, 1 - Monday, etc.
  unsigned char Quarters[SUPLA_WEEKLY_SCHEDULE_VALUES_SIZE / 2];  // 336 B
} TChannelConfig_WeeklySchedule;                                  // v. >= 21

// Config used for thermometers, humidity sensors, and thermometers with
// humidity channels.
// Correction is always applied by io device. Parameter
// AdjustmentAppliedByDevice is added in order to handle older versions where
// correction was applied by server. Devices supporting this setting will
// retreive config from server and if AdjustmentAppliedByDevice is set to 0,
// then they will store new correction value, set AdjustmentAppliedByDevice to 1
// and send it to server, so server will no longer apply correction.
typedef struct {
  _supla_int16_t TemperatureAdjustment;     // * 0.01
  _supla_int16_t HumidityAdjustment;        // * 0.01
  unsigned char AdjustmentAppliedByDevice;  // 1/true - by device
                                            // 0/false - by server
  unsigned char Reserved[27];
} TChannelConfig_TemperatureAndHumidity;  // v. >= 21

// ChannelConfig for all binary sensors (all functions valid for
// SUPLA_CHANNELTYPE_BINARYSENSOR)
// Device doesn't apply this inverted logic on communication towards server.
// It is used only for interanal purposes and for other external interfaces
// like MQTT
// FilteringTimeMs is used to configure how long device should wait for stable
// input signal before changing it's state. If value is set to 0, then field
// is not used by device and server should ignore it. Device may impose minimum
// and maximum values for this field.
typedef struct {
  unsigned char InvertedLogic;              // 0 - not inverted, 1 - inverted
  unsigned _supla_int16_t FilteringTimeMs;  // 0 - not used, > 0 - time in ms
  unsigned char Reserved[29];
} TChannelConfig_BinarySensor;  // v. >= 21

// Not set is set when there is no thermometer for "AUX" available
// at all.
// Disabled is set when thermometer is available (i.e. we can read it and show
// to user), but it is not used by thermostat for any other purpose
// Other values are mainly for UI adjustement (i.e. show temperature as floor,
// as water, generic heater or cooler device)
#define SUPLA_HVAC_AUX_THERMOMETER_TYPE_NOT_SET 0
#define SUPLA_HVAC_AUX_THERMOMETER_TYPE_DISABLED 1
#define SUPLA_HVAC_AUX_THERMOMETER_TYPE_FLOOR 2
#define SUPLA_HVAC_AUX_THERMOMETER_TYPE_WATER 3
#define SUPLA_HVAC_AUX_THERMOMETER_TYPE_GENERIC_HEATER 4
#define SUPLA_HVAC_AUX_THERMOMETER_TYPE_GENERIC_COOLER 5

#define SUPLA_HVAC_ALGORITHM_NOT_SET 0
#define SUPLA_HVAC_ALGORITHM_ON_OFF_SETPOINT_MIDDLE (1ULL << 0)
#define SUPLA_HVAC_ALGORITHM_ON_OFF_SETPOINT_AT_MOST (1ULL << 1)
#define SUPLA_HVAC_ALGORITHM_PID (1ULL << 2)

// HVAC channel validation rules for thermometers:
// - MainThermometerChannelNo must be set
// - AuxThermometerChannelNo is validated and used only when
//     AuxThermometerType != SUPLA_HVAC_AUX_THERMOMETER_TYPE_NOT_SET
// - AuxThermometerChannelNo != MainThermometerChannelNo
// - AuxThermometerChannelNo must be set for
//     SUPLA_CHANNELFNC_HVAC_THERMOSTAT_DIFFERENTIAL. For other functions it is
//     optional.
// - MainThermometerChannelNo and AuxThermometerChannelNo have to be
//     SUPLA_CHANNELTYPE_HUMIDITYANDTEMPSENSOR or SUPLA_CHANNELTYPE_THERMOMETER
// - When AuxThermometerType == SUPLA_HVAC_AUX_THERMOMETER_TYPE_NOT_SET,
//     AuxThermometerChannelNo is ignored, it can be set to 0.

// HVAC channel validation for AntiFreezeAndOverheatProtectionEnabled:
// - function is available for channel functions: HEAT, COOL, HEAT_COOL
// - for other channel functions, this parameter is ignored
// - AntiFreeze/Overheat protection always use MainThermometerChannelNo as
//     temperature source

// HVAC channel validation for Algorithms:
// - AvailableAlgorithms is set only by device
// - UsedAlgorithm == 0 may be reported by device for incorrectly configured
//     thermostat (i.e. there AvailableAlgorithms == 0), this shouldn't happen
//     for devices with proper SW.
// - UsedAlgorithm & AvailableAlgorithms should evaluate to true
// - UsedAlgorithm should contain only one bit set

// MinOnTimeS and MinOffTimeS:
// - function is always available
// - time is given in seconds
// - allowed range: 0 - 600 sec

// OutputValueOnError:
// - function is always available
// - allowed values: -100 .. 100
// - it is recommended to use only -100 (cool), 0 (off), 100 (heat).
// - info: this range comes from assumption that cooling and heating may
//     be enabled with gradients (i.e. for some thermostat with PWM output), but
//     majority of thermostats are on/off, so only -100 (cool), 0 (off), and 100
//     (heat) are proposed here.

// Temperature validation rules:
// - Temperature in "Room Constrain" means:
//     TEMPERATURE_ROOM_MIN <= t <= TEMPERATURE_ROOM_MAX
// - Temperature in "Aux Constrain" means:
//     TEMPERATURE_AUX_MIN <= t <= TEMPERATURE_AUX_MAX
// - Temperatures (t_min, t_max) in "Auto Constrain" means:
//     TEMPERATURE_ROOM_MIN <= t_min <= TEMPERATURE_ROOM_MAX AND
//     TEMPERATURE_ROOM_MIN <= t_max <= TEMPERATURE_ROOM_MAX AND
//     (t_max - t_min >= TEMPERATURE_HEAT_COOL_OFFSET_MIN) AND
//     (t_max - t_min <= TEMPERATURE_HEAT_COOL_OFFSET_MAX)

// TEMPERATURE_FREEZE_PROTECTION - has to be in Room Constrain when
//   AntiFreezeAndOverheatProtectionEnabled is set
// TEMPERATURE_ECO - has to be in Room Constrain
// TEMPERATURE_COMFORT - has to be in Room Constrain
// TEMPERATURE_BOOST - has to be in Room Constrain
// TEMPERATURE_HEAT_PROTECTION - has to be in Room Constrain when function
//   is COOL or HEAT_COOL
// TEMPERATURE_HISTERESIS and TEMPERATURE_AUX_HISTERESIS - has to be
//   TEMPERATURE_HISTERESIS_MIN <= t <= TEMPERATURE_HISTERESIS_MAX
// TEMPERATURE_BELOW_ALARM - has to be in Room Constrain
// TEMPERATURE_ABOVE_ALARM - has to be in Room Constrain
// TEMPERATURE_AUX_MIN_SETPOINT - has to be in Aux Constrain and has
//   to be < TEMPERATURE_AUX_MAX_SETPOINT
// TEMPERATURE_AUX_MAX_SETPOINT - has to be in Aux Constrain and has
//   to be > TEMPERATURE_AUX_MIN_SETPOINT

// Below values are readonly and defines device capabilities for current
// function:
// TEMPERATURE_ROOM_MIN < TEMPERATURE_ROOM_MAX
// TEMPERATURE_AUX_MIN < TEMPERATURE_AUX_MAX
// TEMPERATURE_HISTERESIS_MIN < TEMPERATURE_HISTERESIS_MAX
// TEMPERATURE_HEAT_COOL_OFFSET_MIN < TEMPERATURE_HEAT_COOL_OFFSET_MAX

// Subfunction for SUPLA_CHANNELFNC_HVAC_THERMOSTAT
// Other channel functions dont' use subfunction setting (yet)
#define SUPLA_HVAC_SUBFUNCTION_NOT_SET 0
#define SUPLA_HVAC_SUBFUNCTION_HEAT 1
#define SUPLA_HVAC_SUBFUNCTION_COOL 2

#define SUPLA_HVAC_TEMPERATURE_CONTROL_TYPE_NOT_SUPPORTED 0
#define SUPLA_HVAC_TEMPERATURE_CONTROL_TYPE_ROOM_TEMPERATURE 1
#define SUPLA_HVAC_TEMPERATURE_CONTROL_TYPE_AUX_HEATER_COOLER_TEMPERATURE 2

typedef struct {
  unsigned _supla_int_t MainThermometerChannelNoReadonly : 1;
  unsigned _supla_int_t MainThermometerChannelNoHidden : 1;
  unsigned _supla_int_t AuxThermometerChannelNoReadonly : 1;
  unsigned _supla_int_t AuxThermometerChannelNoHidden : 1;
  unsigned _supla_int_t BinarySensorChannelNoReadonly : 1;
  unsigned _supla_int_t BinarySensorChannelNoHidden : 1;
  unsigned _supla_int_t AuxThermometerTypeReadonly : 1;
  unsigned _supla_int_t AuxThermometerTypeHidden : 1;
  unsigned _supla_int_t AntiFreezeAndOverheatProtectionEnabledReadonly : 1;
  unsigned _supla_int_t AntiFreezeAndOverheatProtectionEnabledHidden : 1;
  unsigned _supla_int_t UsedAlgorithmReadonly : 1;
  unsigned _supla_int_t UsedAlgorithmHidden : 1;
  unsigned _supla_int_t MinOnTimeSReadonly : 1;
  unsigned _supla_int_t MinOnTimeSHidden : 1;
  unsigned _supla_int_t MinOffTimeSReadonly : 1;
  unsigned _supla_int_t MinOffTimeSHidden : 1;
  unsigned _supla_int_t OutputValueOnErrorReadonly : 1;
  unsigned _supla_int_t OutputValueOnErrorHidden : 1;
  unsigned _supla_int_t SubfunctionReadonly : 1;
  unsigned _supla_int_t SubfunctionHidden : 1;
  unsigned _supla_int_t
      TemperatureSetpointChangeSwitchesToManualModeReadonly : 1;
  unsigned _supla_int_t TemperatureSetpointChangeSwitchesToManualModeHidden : 1;
  unsigned _supla_int_t AuxMinMaxSetpointEnabledReadonly : 1;
  unsigned _supla_int_t AuxMinMaxSetpointEnabledHidden : 1;
  unsigned _supla_int_t UseSeparateHeatCoolOutputsReadonly : 1;
  unsigned _supla_int_t UseSeparateHeatCoolOutputsHidden : 1;
  unsigned _supla_int_t TemperaturesFreezeProtectionReadonly : 1;
  unsigned _supla_int_t TemperaturesFreezeProtectionHidden : 1;
  unsigned _supla_int_t TemperaturesEcoReadonly : 1;
  unsigned _supla_int_t TemperaturesEcoHidden : 1;
  unsigned _supla_int_t TemperaturesComfortReadonly : 1;
  unsigned _supla_int_t TemperaturesComfortHidden : 1;
  unsigned _supla_int_t TemperaturesBoostReadonly : 1;
  unsigned _supla_int_t TemperaturesBoostHidden : 1;
  unsigned _supla_int_t TemperaturesHeatProtectionReadonly : 1;
  unsigned _supla_int_t TemperaturesHeatProtectionHidden : 1;
  unsigned _supla_int_t TemperaturesHisteresisReadonly : 1;
  unsigned _supla_int_t TemperaturesHisteresisHidden : 1;
  unsigned _supla_int_t TemperaturesBelowAlarmReadonly : 1;
  unsigned _supla_int_t TemperaturesBelowAlarmHidden : 1;
  unsigned _supla_int_t TemperaturesAboveAlarmReadonly : 1;
  unsigned _supla_int_t TemperaturesAboveAlarmHidden : 1;
  unsigned _supla_int_t TemperaturesAuxMinSetpointReadonly : 1;
  unsigned _supla_int_t TemperaturesAuxMinSetpointHidden : 1;
  unsigned _supla_int_t TemperaturesAuxMaxSetpointReadonly : 1;
  unsigned _supla_int_t TemperaturesAuxMaxSetpointHidden : 1;
  unsigned _supla_int_t MasterThermostatChannelNoReadonly : 1;
  unsigned _supla_int_t MasterThermostatChannelNoHidden : 1;
  unsigned _supla_int_t HeatOrColdSourceSwitchReadonly : 1;
  unsigned _supla_int_t HeatOrColdSourceSwitchHidden : 1;
  unsigned _supla_int_t PumpSwitchReadonly : 1;
  unsigned _supla_int_t PumpSwitchHidden : 1;
  unsigned _supla_int_t TemperaturesAuxHisteresisReadonly : 1;
  unsigned _supla_int_t TemperaturesAuxHisteresisHidden : 1;
  unsigned _supla_int_t Reserved : 10;
} HvacParameterFlags;

typedef struct {
  union {
    _supla_int_t MainThermometerChannelId;
    // Channel numbers for thermometer config. Channels have to be local and
    // numbering is the same as for registration message
    unsigned char MainThermometerChannelNo;
  };

  union {
    _supla_int_t AuxThermometerChannelId;
    unsigned char
        AuxThermometerChannelNo;  // If the channel number points to itself, it
                                  // means that the aux thermometer is not set.
  };

  union {
    _supla_int_t BinarySensorChannelId;
    unsigned char
        BinarySensorChannelNo;  // If the channel number points to itself, it
                                // means that the binary sensor is not set.
  };

  // SUPLA_HVAC_AUX_THERMOMETER_TYPE_
  unsigned char AuxThermometerType;
  unsigned char AntiFreezeAndOverheatProtectionEnabled;
  // bit map SUPLA_HVAC_ALGORITHM_ (readonly)
  unsigned _supla_int16_t AvailableAlgorithms;
  // only one value of SUPLA_HVAC_ALGORITHM_
  unsigned _supla_int16_t UsedAlgorithm;
  // Below Min TimeS parameters defines minimum time of relay/output to be
  // be disabled or enabled in seconds. It is used to prevent to frequent relay
  // state change.
  // Allowed values are 0-600 (10 minutes)
  unsigned _supla_int16_t MinOnTimeS;   // minimum allowed time for output to
                                        // be enabled
  unsigned _supla_int16_t MinOffTimeS;  // minimum allowed time for output to
                                        // be disabled
  signed char OutputValueOnError;       // -100 cool, 0 off (default), 100 heat
  unsigned char Subfunction;            // SUPLA_HVAC_SUBFUNCTION_
  unsigned char TemperatureSetpointChangeSwitchesToManualMode;  // 0 - off,
                                                                // 1 - on (def)
  unsigned char AuxMinMaxSetpointEnabled;  // 0 - off (default), 1 - on
  // For HEAT_COOL thermostats we have two outpus. They can either use
  // shared output for heating/cooling action and second output for heat vs
  // cool mode selection, or they can use separate outputs - one for heating
  // and one for cooling
  unsigned char UseSeparateHeatCoolOutputs;  // 0 - off (default), 1 - on
  HvacParameterFlags ParameterFlags;

  union {
    _supla_int_t MasterThermostatChannelId;
    struct {
      unsigned char MasterThermostatIsSet;  // 0 - no; 1 - yes
      unsigned char MasterThermostatChannelNo;
    };  // v. >= 25
  };

  union {
    _supla_int_t HeatOrColdSourceSwitchChannelId;
    struct {
      unsigned char HeatOrColdSourceSwitchIsSet;  // 0 - no; 1 - yes
      unsigned char HeatOrColdSourceSwitchChannelNo;
    };  // v. >= 25
  };

  union {
    _supla_int_t PumpSwitchChannelId;
    struct {
      unsigned char PumpSwitchIsSet;  // 0 - no; 1 - yes
      unsigned char PumpSwitchChannelNo;
    };  // v. >= 25
  };

  // TemperatureControlType allows to switch between work based on main
  // thermometer (room) and aux thermometer (heater/cooler).
  // Option is available only for SUPLA_CHANNELFNC_HVAC_THERMOSTAT
  // If set to 0, then it is not supported.
  unsigned char TemperatureControlType;  // SUPLA_HVAC_TEMPERATURE_CONTROL_TYPE_

  unsigned char Reserved[48 - sizeof(HvacParameterFlags) -
                         sizeof(_supla_int_t) - sizeof(_supla_int_t) -
                         sizeof(_supla_int_t) - sizeof(unsigned char)];
  THVACTemperatureCfg Temperatures;
} TChannelConfig_HVAC;  // v. >= 21

#define SUPLA_GENERAL_PURPOSE_MEASUREMENT_CHART_TYPE_LINEAR 0
#define SUPLA_GENERAL_PURPOSE_MEASUREMENT_CHART_TYPE_BAR 1
#define SUPLA_GENERAL_PURPOSE_MEASUREMENT_CHART_TYPE_CANDLE 2

#define SUPLA_GENERAL_PURPOSE_UNIT_SIZE 15

// General Purpose Mesurement channel config:
// Calculated value is: (value / ValueDivider) * ValueMultiplier + ValueAdded
typedef struct {
  // Value divider
  _supla_int_t ValueDivider;  // 0.001 units; 0 is considered as 1
  // Value multiplier
  _supla_int_t ValueMultiplier;  // 0.001 units; 0 is considered as 1
  // Value added
  _supla_int64_t ValueAdded;  // 0.001 units
  // Display precicion
  unsigned char ValuePrecision;  // 0 - 4 decimal points
  // Display unit (before value) - UTF8 including the terminating null byte '\0'
  char UnitBeforeValue[SUPLA_GENERAL_PURPOSE_UNIT_SIZE];
  // Display unit (after value) - UTF8 including the terminating null byte '\0'
  char UnitAfterValue[SUPLA_GENERAL_PURPOSE_UNIT_SIZE];
  unsigned char
      NoSpaceBeforeValue;  // 0 - with space (default), 1 - without space
  unsigned char
      NoSpaceAfterValue;  // 0 - with space (default), 1 - without space
  // Keep history on server
  unsigned char KeepHistory;  // 0 - no (default), 1 - yes
  // Chart type linear/bar/candle
  unsigned char ChartType;  // SUPLA_GENERAL_PURPOSE_MEASUREMENT_CHART_TYPE_*
  // Channel refresh interval [ms], allowed range 1 .. 65535
  // Actual channel implementation may enforce minimum value
  // 0 - default - implementation dependent
  unsigned _supla_int16_t RefreshIntervalMs;

  // Readonly and default parameters
  // Default value divider
  _supla_int_t DefaultValueDivider;  // 0.001 units; 0 is considered as 1
  // Default value multiplier
  _supla_int_t DefaultValueMultiplier;  // 0.001 units; 0 is considered as 1
  // Default value added
  _supla_int64_t DefaultValueAdded;  // 0.001 units
  // Default display precicion
  unsigned char DefaultValuePrecision;  // 0 - 4 decimal points
  // Default unit (before value) - UTF8 including the terminating null byte '\0'
  char DefaultUnitBeforeValue[SUPLA_GENERAL_PURPOSE_UNIT_SIZE];
  // Default unit (after value) - UTF8 including the terminating null byte '\0'
  char DefaultUnitAfterValue[SUPLA_GENERAL_PURPOSE_UNIT_SIZE];

  unsigned char Reserved[8];
} TChannelConfig_GeneralPurposeMeasurement;  // v. >= 23

#define SUPLA_GENERAL_PURPOSE_METER_COUNTER_TYPE_INCREMENT_DECREMENT 0
#define SUPLA_GENERAL_PURPOSE_METER_COUNTER_TYPE_ALWAYS_INCREMENT 1
#define SUPLA_GENERAL_PURPOSE_METER_COUNTER_TYPE_ALWAYS_DECREMENT 2

#define SUPLA_GENERAL_PURPOSE_METER_CHART_TYPE_BAR 0
#define SUPLA_GENERAL_PURPOSE_METER_CHART_TYPE_LINEAR 1

// General Purpose Meter channel config:
// Calculated value is: (value / ValueDivider) * ValueMultiplier + ValueAdded
typedef struct {
  // Counter type
  unsigned char CounterType;  // SUPLA_GENERAL_PURPOSE_METER_COUNTER_TYPE_*

  // Value divider
  _supla_int_t ValueDivider;  // 0.001 units; 0 is considered as 1
  // Value multiplier
  _supla_int_t ValueMultiplier;  // 0.001 units; 0 is considered as 1
  // Value added
  _supla_int64_t ValueAdded;  // 0.001 units
  // Display precicion
  unsigned char ValuePrecision;  // 0 - 4 decimal points
  // Display unit (before value) - UTF8 including the terminating null byte '\0'
  char UnitBeforeValue[SUPLA_GENERAL_PURPOSE_UNIT_SIZE];
  // Display unit (after value) - UTF8 including the terminating null byte '\0'
  char UnitAfterValue[SUPLA_GENERAL_PURPOSE_UNIT_SIZE];
  unsigned char
      NoSpaceBeforeValue;  // 0 - with space (default), 1 - without space
  unsigned char
      NoSpaceAfterValue;  // 0 - with space (default), 1 - without space
  // Keep history on server
  unsigned char KeepHistory;  // 0 - no (default), 1 - yes
  // Chart type linear/bar/candle
  unsigned char ChartType;  // SUPLA_GENERAL_PURPOSE_METER_CHART_TYPE_*
  // Include value added in history
  unsigned char IncludeValueAddedInHistory;  // 0 - no (default), 1 - yes
  // Fill missing data
  unsigned char FillMissingData;  // 0 - no (default), 1 - yes
  // Channel refresh interval [ms], allowed range 1 .. 65535
  // Actual channel implementation may enforce minimum value
  // 0 - default - implementation dependent
  unsigned _supla_int16_t RefreshIntervalMs;

  // Readonly and default parameters
  // Default value divider
  _supla_int_t DefaultValueDivider;  // 0.001 units; 0 is considered as 1
  // Default value multiplier
  _supla_int_t DefaultValueMultiplier;  // 0.001 units; 0 is considered as 1
  // Default value added
  _supla_int64_t DefaultValueAdded;  // 0.001 units
  // Default display precicion
  unsigned char DefaultValuePrecision;  // 0 - 4 decimal points
  // Default unit (before value) - UTF8 including the terminating null byte '\0'
  char DefaultUnitBeforeValue[SUPLA_GENERAL_PURPOSE_UNIT_SIZE];
  // Default unit (after value) - UTF8 including the terminating null byte '\0'
  char DefaultUnitAfterValue[SUPLA_GENERAL_PURPOSE_UNIT_SIZE];

  unsigned char Reserved[8];
} TChannelConfig_GeneralPurposeMeter;  // v. >= 23

#define EM_CT_TYPE_100A_33mA (1ULL << 0)
#define EM_CT_TYPE_200A_66mA (1ULL << 1)
#define EM_CT_TYPE_400A_133mA (1ULL << 2)

#define EM_PHASE_LED_TYPE_OFF (1ULL << 0)
#define EM_PHASE_LED_TYPE_VOLTAGE_PRESENCE (1ULL << 1)
#define EM_PHASE_LED_TYPE_VOLTAGE_PRESENCE_INVERTED (1ULL << 2)
// Voltage level:
//  - PhaseLedParam1 - "low threshold", units 0.01 V.
//    When voltage < low threshold -> blue LED
//  - PhaseLedParam2 - "high threshold", units 0.01 V.
//    When voltage > high threshold -> red LED
//  - Voltage between low and high threshold -> green LED
#define EM_PHASE_LED_TYPE_VOLTAGE_LEVEL (1ULL << 3)
// Active power direction:
//  - PhaseLedParam1 - "low threshold", units 0.01 W.
//    When power < low threshold -> green LED.
//  - PhaseLedParam2 - "high threshold", units 0.01 W.
//    When power > high threshold -> red LED
#define EM_PHASE_LED_TYPE_POWER_ACTIVE_DIRECTION (1ULL << 4)

// Electricity meter channel config
typedef struct {
  // Selected CT type
  unsigned _supla_int64_t UsedCTType;  // EM_CT_TYPE_
  // Selected phase LED type
  unsigned _supla_int64_t UsedPhaseLedType;  // EM_PHASE_LED_TYPE_*

  // Phase LED params (actual meaning depends on phase LED type)
  _supla_int_t PhaseLedParam1;
  _supla_int_t PhaseLedParam2;

  // readonly, device capabilities
  unsigned _supla_int64_t AvailableCTTypes;
  unsigned _supla_int64_t AvailablePhaseLedTypes;

  unsigned char Reserved[32];
} TChannelConfig_ElectricityMeter;  // v. >= 23

typedef struct {
  _supla_int_t PricePerUnit;  // * 0.0001
  // Currency Code A https://www.nationsonline.org/oneworld/currencies.htm
  char Currency[3];
  char CustomUnit[9];  // UTF8 including the terminating null byte ('\0')

  _supla_int_t ImpulsesPerUnit;
  _supla_int64_t InitialValue;  // 0.001 units
  unsigned char AddToHistory;   // 0 - False, 1 - True

  unsigned char Reserved[32];
} TChannelConfig_ImpulseCounter;  // v. >= 25

#define SUPLA_OCR_AUTHKEY_SIZE 33

#define OCR_LIGHTING_MODE_OFF (1ULL << 0)
#define OCR_LIGHTING_MODE_ALWAYS_ON (1ULL << 1)
#define OCR_LIGHTING_MODE_AUTO (1ULL << 2)

typedef struct {
  char AuthKey[SUPLA_OCR_AUTHKEY_SIZE];  // Set by the server. Alphanumeric null
                                         // terminated string.
  char Host[SUPLA_URL_HOST_MAXSIZE];     // Set by the server. Including the
                                         // terminating null byte ('\0').

  unsigned _supla_int_t
      PhotoIntervalSec;  // 0 - Disabled. The server may discard the
                         // value if it considers the frequency to be
                         // too high or too low. The server can set
                         // the accepted value.
  unsigned _supla_int64_t LightingMode;      // OCR_LIGHTING_MODE *
  unsigned char LightingLevel;               // 1-100%
  unsigned _supla_int64_t MaximumIncrement;  // Maximum impulse increment
                                             // between shots. 0 == Unspecified

  // readonly, device capabilities
  unsigned _supla_int64_t AvailableLightingModes;
  unsigned char Reserved[128];
} TChannelConfig_OCR;  // v. >= 25

typedef struct {
  // If OvercurrentMaxAllowed == 0, then overcurrent settings are not available.
  // If OvercurrentThreshold == 0, then overcurrent protection is disabled.
  unsigned _supla_int_t OvercurrentThreshold;   // in 0.01 A
  unsigned _supla_int_t OvercurrentMaxAllowed;  // in 0.01 A, readonly
  unsigned char DefaultRelatedMeterIsSet;       // readonly, 1 - true, 0 - false
  unsigned char
      DefaultRelatedMeterChannelNo;  // readonly, provides channel number of
                                     // related meter if RelatedMeterIsSet
  unsigned char Reserved[32];
} TChannelConfig_PowerSwitch;  // v. >= 25

typedef TChannelConfig_PowerSwitch TChannelConfig_LightSwitch;

typedef struct {
  _supla_int_t ChannelID;
  union {
    unsigned _supla_int_t DurationMS;
    unsigned _supla_int_t DurationSec;  // ver. >= 21. Applies to HVAC
  };
  unsigned char On;
} TCS_TimerArmRequest;  // v. >= 17

// Recommended for bistable buttons
#define SUPLA_ACTION_CAP_TURN_ON (1 << 0)
#define SUPLA_ACTION_CAP_TURN_OFF (1 << 1)
#define SUPLA_ACTION_CAP_TOGGLE_x1 (1 << 2)
#define SUPLA_ACTION_CAP_TOGGLE_x2 (1 << 3)
#define SUPLA_ACTION_CAP_TOGGLE_x3 (1 << 4)
#define SUPLA_ACTION_CAP_TOGGLE_x4 (1 << 5)
#define SUPLA_ACTION_CAP_TOGGLE_x5 (1 << 6)

// Recommended for monostable buttons
#define SUPLA_ACTION_CAP_HOLD (1 << 10)
#define SUPLA_ACTION_CAP_SHORT_PRESS_x1 (1 << 11)
#define SUPLA_ACTION_CAP_SHORT_PRESS_x2 (1 << 12)
#define SUPLA_ACTION_CAP_SHORT_PRESS_x3 (1 << 13)
#define SUPLA_ACTION_CAP_SHORT_PRESS_x4 (1 << 14)
#define SUPLA_ACTION_CAP_SHORT_PRESS_x5 (1 << 15)

#define SUPLA_VALVE_FLAG_FLOODING 0x1
#define SUPLA_VALVE_FLAG_MANUALLY_CLOSED 0x2

typedef struct {
  union {
    unsigned char closed;
    unsigned char closed_percent;
  };

  unsigned char flags;
} TValve_Value;

typedef struct {
  unsigned char ChannelNumber;
  _supla_int_t ActionTrigger;
  unsigned char zero[10];  // Place for future variables
} TDS_ActionTrigger;

#define SUPLA_PN_TITLE_MAXSIZE 101
#define SUPLA_PN_BODY_MAXSIZE 256
#define SUPLA_PN_PROFILE_NAME_MAXSIZE 51

#define PN_SERVER_MANAGED_TITLE (1 << 0)
#define PN_SERVER_MANAGED_BODY (1 << 1)
#define PN_SERVER_MANAGED_SOUND (1 << 2)

typedef struct {
  unsigned char
      ServerManagedFields;  // Specifies the fields that will be managed by the
                            // server. This value is only considered for the
                            // first notification within a given context and
                            // applies to all subsequent notifications.

  _supla_int16_t Context;  // >= 0 Channel, -1 Device
  signed char Reserved[8];
} TDS_RegisterPushNotification;

typedef struct {
  _supla_int16_t Context;  // >= 0 Channel, -1 Device
  signed char Reserved[32];
  _supla_int_t SoundId;
  unsigned _supla_int16_t
      TitleSize;  // Including the terminating null byte ('\0').
  unsigned _supla_int16_t
      BodySize;  // Including the terminating null byte ('\0').
  signed char TitleAndBody[SUPLA_PN_TITLE_MAXSIZE +
                           SUPLA_PN_BODY_MAXSIZE];  // Last variable in struct!
} TDS_PushNotification;

#define SUPLA_SUBDEVICE_PRODUCT_CODE_MAXSIZE 51
#define SUPLA_SUBDEVICE_SERIAL_NUMBER_MAXSIZE 51

typedef struct {
  // device -> server
  unsigned char SubDeviceId;

  char Name[SUPLA_DEVICE_NAME_MAXSIZE];  // UTF8 - 201 B including the
                                         // terminating null byte ('\0').
  char SoftVer[SUPLA_SOFTVER_MAXSIZE];   //  21 B including the terminating null
                                         //  byte ('\0').
  char ProductCode[SUPLA_SUBDEVICE_PRODUCT_CODE_MAXSIZE];  // 51 B including the
                                                           // terminating null
                                                           // byte ('\0').
  char
      SerialNumber[SUPLA_SUBDEVICE_SERIAL_NUMBER_MAXSIZE];  // 51 B including
                                                            // the terminating
                                                            // null byte ('\0').
} TDS_SubdeviceDetails;

#define SUPLA_PN_CLIENT_TOKEN_MAXSIZE 256
#define PLATFORM_UNKNOWN 0
#define PLATFORM_IOS 1
#define PLATFORM_ANDROID 2
#define PLATFORM_HOMEGRAPH 3  // v. >= 23

typedef struct {
  unsigned char DevelopmentEnv;
  _supla_int_t Platform;
  _supla_int_t AppId;
  signed char
      ProfileName[SUPLA_PN_PROFILE_NAME_MAXSIZE];  // Including the terminating
                                                   // null byte ('\0').
  unsigned _supla_int16_t
      RealTokenSize;  // It allows you to determine if the maximum size of the
                      // Token variable is sufficient.
  unsigned _supla_int16_t
      TokenSize;  // Including the terminating null byte ('\0'). Size
                  // <= 1 removes the token
  signed char Token[SUPLA_PN_CLIENT_TOKEN_MAXSIZE];  // Last variable in struct!
} TCS_PnClientToken;

typedef struct {
  TCS_ClientAuthorizationDetails Auth;
  TCS_PnClientToken Token;  // Last variable in struct!
} TCS_RegisterPnClientToken;

typedef struct {
  _supla_int_t ResultCode;
} TSC_RegisterPnClientTokenResult;

#pragma pack(pop)

void *PROTO_ICACHE_FLASH sproto_init(void);
void PROTO_ICACHE_FLASH sproto_free(void *spd_ptr);

#ifndef SPROTO_WITHOUT_OUT_BUFFER
char PROTO_ICACHE_FLASH sproto_out_buffer_append(void *spd_ptr,
                                                 TSuplaDataPacket *sdp);
unsigned _supla_int_t sproto_pop_out_data(void *spd_ptr, char *buffer,
                                          unsigned _supla_int_t buffer_size);
#endif /*SPROTO_WITHOUT_OUT_BUFFER*/
char PROTO_ICACHE_FLASH sproto_out_dataexists(void *spd_ptr);
char PROTO_ICACHE_FLASH sproto_in_buffer_append(
    void *spd_ptr, char *data, unsigned _supla_int_t data_size);

char PROTO_ICACHE_FLASH sproto_pop_in_sdp(void *spd_ptr, TSuplaDataPacket *sdp);
char PROTO_ICACHE_FLASH sproto_in_dataexists(void *spd_ptr);

unsigned char PROTO_ICACHE_FLASH sproto_get_version(void *spd_ptr);
void PROTO_ICACHE_FLASH sproto_set_version(void *spd_ptr,
                                           unsigned char version);
void PROTO_ICACHE_FLASH sproto_sdp_init(void *spd_ptr, TSuplaDataPacket *sdp);
char PROTO_ICACHE_FLASH sproto_set_data(TSuplaDataPacket *sdp, char *data,
                                        unsigned _supla_int_t data_size,
                                        unsigned _supla_int_t call_id);
TSuplaDataPacket *PROTO_ICACHE_FLASH sproto_sdp_malloc(void *spd_ptr);
void PROTO_ICACHE_FLASH sproto_sdp_free(TSuplaDataPacket *sdp);

void PROTO_ICACHE_FLASH sproto_log_summary(void *spd_ptr);
void PROTO_ICACHE_FLASH sproto_buffer_dump(void *spd_ptr, unsigned char in);

void PROTO_ICACHE_FLASH sproto_set_null_terminated_string(
    const char *src, char *dest, unsigned _supla_int_t *dest_size,
    unsigned int max_size);

void PROTO_ICACHE_FLASH sproto__set_null_terminated_string(
    const char *src, char *dest, unsigned _supla_int16_t *dest_size,
    unsigned int max_size);

#ifdef __cplusplus
}
#endif

#endif /* supla_proto_H_ */
