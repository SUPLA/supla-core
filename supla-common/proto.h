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

#include <WinSock2.h>
#define _supla_int_t int
#define _supla_int16_t short
#define _supla_int64_t __int64
#define _supla_timeval timeval

#elif defined(__AVR__)

#define SPROTO_WITHOUT_OUT_BUFFER

struct _supla_timeval {
  long tv_sec[2];
  long tv_usec[2];
};

#define timeval _supla_timeval

#define _supla_int16_t int
#define _supla_int_t long
#define _supla_int64_t long long

#elif defined(ESP8266) || defined(ESP32)
#include <mem.h>
#define PROTO_ICACHE_FLASH ICACHE_FLASH_ATTR

#if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)
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

struct _supla_timeval {
  long long tv_sec;
  long long tv_usec;
};

#include <sys/time.h>
#define _supla_int16_t short
#define _supla_int_t int
#define _supla_int64_t long long

#else /*__arm__*/
#include <sys/time.h>
#define _supla_int16_t short
#define _supla_int_t int
#define _supla_int64_t long long
#define _supla_timeval timeval
#endif

#if defined(ARDUINO)
#undef PROTO_ICACHE_FLASH
#endif /*defined(ARDUINO)*/

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

#define SUPLA_PROTO_VERSION 17
#define SUPLA_PROTO_VERSION_MIN 1
#if defined(ARDUINO_ARCH_AVR)     // Arduino IDE for Arduino HW
#define SUPLA_MAX_DATA_SIZE 1248  // Registration header + 32 channels x 21 B
#elif defined(ARDUINO_ARCH_ESP8266) || \
    defined(ARDUINO_ARCH_ESP32)   // Arduino IDE for ESP8266
#define SUPLA_MAX_DATA_SIZE 3264  // Registration header + 128 channels x 21 B
#elif defined(ESP8266)
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
  (SUPLA_MAX_DATA_SIZE - 50)           // ver. >= 10
#define SUPLA_CALCFG_DATA_MAXSIZE 128  // ver. >= 10
#define SUPLA_TIMEZONE_MAXSIZE 51      // ver. >= 11

#ifndef SUPLA_CHANNELGROUP_RELATION_PACK_MAXCOUNT
#define SUPLA_CHANNELGROUP_RELATION_PACK_MAXCOUNT 100  // ver. >= 9
#endif /*SUPLA_CHANNELGROUP_RELATION_PACK_MAXCOUNT*/

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
#define SUPLA_CS_CALL_REGISTER_CLIENT 80
#define SUPLA_CS_CALL_REGISTER_CLIENT_B 85  // ver. >= 6
#define SUPLA_CS_CALL_REGISTER_CLIENT_C 86  // ver. >= 7
#define SUPLA_CS_CALL_REGISTER_CLIENT_D 87  // ver. >= 12
#define SUPLA_SC_CALL_REGISTER_CLIENT_RESULT 90
#define SUPLA_SC_CALL_REGISTER_CLIENT_RESULT_B 92  // ver. >= 9
#define SUPLA_SC_CALL_REGISTER_CLIENT_RESULT_C 94  // ver. >= 17
#define SUPLA_DS_CALL_DEVICE_CHANNEL_VALUE_CHANGED 100
#define SUPLA_DS_CALL_DEVICE_CHANNEL_VALUE_CHANGED_B 102        // ver. >= 12
#define SUPLA_DS_CALL_DEVICE_CHANNEL_VALUE_CHANGED_C 103        // ver. >= 12
#define SUPLA_DS_CALL_DEVICE_CHANNEL_EXTENDEDVALUE_CHANGED 105  // ver. >= 10
#define SUPLA_SD_CALL_CHANNEL_SET_VALUE 110
#define SUPLA_SD_CALL_CHANNELGROUP_SET_VALUE 115  // ver. >= 13
#define SUPLA_DS_CALL_CHANNEL_SET_VALUE_RESULT 120
#define SUPLA_SC_CALL_LOCATION_UPDATE 130
#define SUPLA_SC_CALL_LOCATIONPACK_UPDATE 140
#define SUPLA_SC_CALL_CHANNEL_UPDATE 150
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
#define SUPLA_SC_CALL_CHANNEL_UPDATE_B 370                    // ver. >= 8
#define SUPLA_SC_CALL_CHANNEL_UPDATE_C 371                    // ver. >= 10
#define SUPLA_SC_CALL_CHANNEL_UPDATE_D 372                    // ver. >= 15
#define SUPLA_SC_CALL_CHANNELGROUP_PACK_UPDATE 380            // ver. >= 9
#define SUPLA_SC_CALL_CHANNELGROUP_PACK_UPDATE_B 381          // ver. >= 10
#define SUPLA_SC_CALL_CHANNELGROUP_RELATION_PACK_UPDATE 390   // ver. >= 9
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
#define SUPLA_CS_CALL_SET_CHANNEL_CAPTION 640                 // ver. >= 12
#define SUPLA_SC_CALL_SET_CHANNEL_CAPTION_RESULT 650          // ver. >= 12
#define SUPLA_CS_CALL_SET_LOCATION_CAPTION 645                // ver. >= 14
#define SUPLA_SC_CALL_SET_LOCATION_CAPTION_RESULT 655         // ver. >= 14
#define SUPLA_DS_CALL_GET_CHANNEL_CONFIG 680                  // ver. >= 16
#define SUPLA_SD_CALL_GET_CHANNEL_CONFIG_RESULT 690           // ver. >= 16
#define SUPLA_DS_CALL_ACTIONTRIGGER 700                       // ver. >= 16
#define SUPLA_CS_CALL_TIMER_ARM 800                           // ver. >= 17

#define SUPLA_RESULT_CALL_NOT_ALLOWED -5
#define SUPLA_RESULT_DATA_TOO_LARGE -4
#define SUPLA_RESULT_BUFFER_OVERFLOW -3
#define SUPLA_RESULT_DATA_ERROR -2
#define SUPLA_RESULT_VERSION_ERROR -1
#define SUPLA_RESULT_FALSE 0
#define SUPLA_RESULT_TRUE 1

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
#define SUPLA_RESULTCODE_HOSTNOTFOUND 15                           // ver. >= 5
#define SUPLA_RESULTCODE_CANTCONNECTTOHOST 16                      // ver. >= 5
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
  30  // ver. >= 16

#define SUPLA_OAUTH_RESULTCODE_ERROR 0         // ver. >= 10
#define SUPLA_OAUTH_RESULTCODE_SUCCESS 1       // ver. >= 10
#define SUPLA_OAUTH_TEMPORARILY_UNAVAILABLE 2  // ver. >= 10

#define SUPLA_DEVICE_NAME_MAXSIZE 201
#define SUPLA_CLIENT_NAME_MAXSIZE 201
#define SUPLA_SENDER_NAME_MAXSIZE 201

#ifdef __AVR__
#ifdef __AVR_ATmega2560__
#define SUPLA_CHANNELMAXCOUNT 32
#else
#define SUPLA_CHANNELMAXCOUNT 1
#endif
#else
#define SUPLA_CHANNELMAXCOUNT 128
#endif

#define SUPLA_CHANNELVALUE_SIZE 8

#ifdef __AVR__
#define SUPLA_CHANNELEXTENDEDVALUE_SIZE 256
#else
#define SUPLA_CHANNELEXTENDEDVALUE_SIZE 1024
#endif

#define SUPLA_CHANNELTYPE_SENSORNO 1000
#define SUPLA_CHANNELTYPE_SENSORNC 1010        // DEPRECATED
#define SUPLA_CHANNELTYPE_DISTANCESENSOR 1020  // ver. >= 5
#define SUPLA_CHANNELTYPE_CALLBUTTON 1500      // ver. >= 4
#define SUPLA_CHANNELTYPE_RELAYHFD4 2000
#define SUPLA_CHANNELTYPE_RELAYG5LA1A 2010
#define SUPLA_CHANNELTYPE_2XRELAYG5LA1A 2020
#define SUPLA_CHANNELTYPE_RELAY 2900
#define SUPLA_CHANNELTYPE_THERMOMETERDS18B20 3000
#define SUPLA_CHANNELTYPE_DHT11 3010   // ver. >= 4
#define SUPLA_CHANNELTYPE_DHT22 3020   // ver. >= 4
#define SUPLA_CHANNELTYPE_DHT21 3022   // ver. >= 5
#define SUPLA_CHANNELTYPE_AM2302 3030  // ver. >= 4
#define SUPLA_CHANNELTYPE_AM2301 3032  // ver. >= 5

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

#define SUPLA_CHANNELTYPE_VALVE_OPENCLOSE 7000              // ver. >= 12
#define SUPLA_CHANNELTYPE_VALVE_PERCENTAGE 7010             // ver. >= 12
#define SUPLA_CHANNELTYPE_BRIDGE 8000                       // ver. >= 12
#define SUPLA_CHANNELTYPE_GENERAL_PURPOSE_MEASUREMENT 9000  // ver. >= 12
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
#define SUPLA_CHANNELFNC_DEPTHSENSOR 210                  // ver. >= 5
#define SUPLA_CHANNELFNC_DISTANCESENSOR 220               // ver. >= 5
#define SUPLA_CHANNELFNC_OPENINGSENSOR_WINDOW 230         // ver. >= 8
#define SUPLA_CHANNELFNC_MAILSENSOR 240                   // ver. >= 8
#define SUPLA_CHANNELFNC_WINDSENSOR 250                   // ver. >= 8
#define SUPLA_CHANNELFNC_PRESSURESENSOR 260               // ver. >= 8
#define SUPLA_CHANNELFNC_RAINSENSOR 270                   // ver. >= 8
#define SUPLA_CHANNELFNC_WEIGHTSENSOR 280                 // ver. >= 8
#define SUPLA_CHANNELFNC_WEATHER_STATION 290              // ver. >= 8
#define SUPLA_CHANNELFNC_STAIRCASETIMER 300               // ver. >= 8
#define SUPLA_CHANNELFNC_ELECTRICITY_METER 310            // ver. >= 10
#define SUPLA_CHANNELFNC_IC_ELECTRICITY_METER 315         // ver. >= 12
#define SUPLA_CHANNELFNC_IC_GAS_METER 320                 // ver. >= 10
#define SUPLA_CHANNELFNC_IC_WATER_METER 330               // ver. >= 10
#define SUPLA_CHANNELFNC_IC_HEAT_METER 340                // ver. >= 10
#define SUPLA_CHANNELFNC_THERMOSTAT 400                   // ver. >= 11
#define SUPLA_CHANNELFNC_THERMOSTAT_HEATPOL_HOMEPLUS 410  // ver. >= 11
#define SUPLA_CHANNELFNC_VALVE_OPENCLOSE 500              // ver. >= 12
#define SUPLA_CHANNELFNC_VALVE_PERCENTAGE 510             // ver. >= 12
#define SUPLA_CHANNELFNC_GENERAL_PURPOSE_MEASUREMENT 520  // ver. >= 12
#define SUPLA_CHANNELFNC_CONTROLLINGTHEENGINESPEED 600    // ver. >= 12
#define SUPLA_CHANNELFNC_ACTIONTRIGGER 700                // ver. >= 16
#define SUPLA_CHANNELFNC_DIGIGLASS_HORIZONTAL 800         // ver. >= 14
#define SUPLA_CHANNELFNC_DIGIGLASS_VERTICAL 810           // ver. >= 14

#define SUPLA_BIT_FUNC_CONTROLLINGTHEGATEWAYLOCK 0x00000001
#define SUPLA_BIT_FUNC_CONTROLLINGTHEGATE 0x00000002
#define SUPLA_BIT_FUNC_CONTROLLINGTHEGARAGEDOOR 0x00000004
#define SUPLA_BIT_FUNC_CONTROLLINGTHEDOORLOCK 0x00000008
#define SUPLA_BIT_FUNC_CONTROLLINGTHEROLLERSHUTTER 0x00000010
#define SUPLA_BIT_FUNC_POWERSWITCH 0x00000020
#define SUPLA_BIT_FUNC_LIGHTSWITCH 0x00000040
#define SUPLA_BIT_FUNC_STAIRCASETIMER 0x00000080            // ver. >= 8
#define SUPLA_BIT_FUNC_THERMOMETER 0x00000100               // ver. >= 12
#define SUPLA_BIT_FUNC_HUMIDITYANDTEMPERATURE 0x00000200    // ver. >= 12
#define SUPLA_BIT_FUNC_HUMIDITY 0x00000400                  // ver. >= 12
#define SUPLA_BIT_FUNC_WINDSENSOR 0x00000800                // ver. >= 12
#define SUPLA_BIT_FUNC_PRESSURESENSOR 0x00001000            // ver. >= 12
#define SUPLA_BIT_FUNC_RAINSENSOR 0x00002000                // ver. >= 12
#define SUPLA_BIT_FUNC_WEIGHTSENSOR 0x00004000              // ver. >= 12
#define SUPLA_BIT_FUNC_CONTROLLINGTHEROOFWINDOW 0x00008000  // ver. >= 13

#define SUPLA_EVENT_CONTROLLINGTHEGATEWAYLOCK 10
#define SUPLA_EVENT_CONTROLLINGTHEGATE 20
#define SUPLA_EVENT_CONTROLLINGTHEGARAGEDOOR 30
#define SUPLA_EVENT_CONTROLLINGTHEDOORLOCK 40
#define SUPLA_EVENT_CONTROLLINGTHEROLLERSHUTTER 50
#define SUPLA_EVENT_CONTROLLINGTHEROOFWINDOW 55
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

#define SUPLA_CHANNEL_FLAG_ZWAVE_BRIDGE 0x0001  // ver. >= 12
#define SUPLA_CHANNEL_FLAG_IR_BRIDGE 0x0002     // ver. >= 12
#define SUPLA_CHANNEL_FLAG_RF_BRIDGE 0x0004     // ver. >= 12
// Free bit for future use: 0x0008
#define SUPLA_CHANNEL_FLAG_CHART_TYPE_BAR 0x0010                  // ver. >= 12
#define SUPLA_CHANNEL_FLAG_CHART_DS_TYPE_DIFFERENTAL 0x0020       // ver. >= 12
#define SUPLA_CHANNEL_FLAG_CHART_INTERPOLATE_MEASUREMENTS 0x0040  // ver. >= 12
// Free bits for future use:  0x0080, 0x0100, 0x0200, 0x0400, 0x0800
#define SUPLA_CHANNEL_FLAG_RS_AUTO_CALIBRATION 0x1000    // ver. >= 15
#define SUPLA_CHANNEL_FLAG_CALCFG_RESET_COUNTERS 0x2000  // ver. >= 15
// Free bits for future use: 0x8000
#define SUPLA_CHANNEL_FLAG_CALCFG_RECALIBRATE 0x4000               // ver. >= 15
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
  0x02000000                                               // ver. >= 12
#define SUPLA_CHANNEL_FLAG_POSSIBLE_SLEEP_MODE 0x04000000  // ver. >= 12

#pragma pack(push, 1)

typedef struct {
  char tag[SUPLA_TAG_SIZE];
  unsigned char version;
  unsigned _supla_int_t rr_id;  // Request/Response ID
  unsigned _supla_int_t call_type;
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
#define EV_TYPE_ELECTRICITY_METER_MEASUREMENT_V2 12
#define EV_TYPE_IMPULSE_COUNTER_DETAILS_V1 20
#define EV_TYPE_THERMOSTAT_DETAILS_V1 30
#define EV_TYPE_CHANNEL_STATE_V1 40
#define EV_TYPE_TIMER_STATE_V1 50
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
  };
} TDS_SuplaDeviceChannel_C;  // ver. >= 10

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

  char ServerName[SUPLA_SERVER_NAME_MAXSIZE];

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
  // server -> device

  _supla_int_t result_code;
  unsigned char activity_timeout;
  unsigned char version;
  unsigned char version_min;
} TSD_SuplaRegisterDeviceResult;

typedef struct {
  // device -> server

  unsigned char ChannelNumber;
  char value[SUPLA_CHANNELVALUE_SIZE];
} TDS_SuplaDeviceChannelValue;

typedef struct {
  // device -> server

  unsigned char ChannelNumber;
  unsigned char Offline;
  char value[SUPLA_CHANNELVALUE_SIZE];
} TDS_SuplaDeviceChannelValue_B;  // v. >= 12

typedef struct {
  // device -> server

  unsigned char ChannelNumber;
  unsigned char Offline;
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
  unsigned _supla_int_t DurationMS;

  char value[SUPLA_CHANNELVALUE_SIZE];
} TSD_SuplaChannelNewValue;

typedef struct {
  // server -> device
  _supla_int_t SenderID;
  _supla_int_t GroupID;
  unsigned char EOL;  // End Of List
  unsigned char ChannelNumber;
  unsigned _supla_int_t DurationMS;

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
      current[3];  // * 0.001 A (0.01A FOR EM_VAR_CURRENT_OVER_65A)
  _supla_int_t power_active[3];    // * 0.00001 W or kW
  _supla_int_t power_reactive[3];  // * 0.00001 var or kvar
  _supla_int_t power_apparent[3];  // * 0.00001 VA or kVA
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
#define EM_VAR_ALL 0xFFFF

#define EM_VAR_POWER_ACTIVE_KWH 0x100000
#define EM_VAR_POWER_REACTIVE_KVAR 0x200000
#define EM_VAR_POWER_APPARENT_KVA 0x400000

#ifdef __AVR__
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

typedef struct {
  char position;  // -1 == calibration. -1 - 100%
  char tilt;
  char bottom_position;  // Percentage points to the windowsill
  _supla_int16_t flags;
} TRollerShutterValue;

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

#define SUPLA_CALCFG_DATATYPE_RS_SETTINGS 1000

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
  unsigned char ResetCounter;          // 0 - NO, 1 - YES
  unsigned char SetTime;               // 0 - NO, 1 - YES
  unsigned short LightSourceLifespan;  // 0 - 65535 hours
} TCalCfg_LightSourceLifespan;

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

typedef struct {
  _supla_int_t FullOpeningTimeMS;
  _supla_int_t FullClosingTimeMS;
} TCalCfg_RollerShutterSettings;

#define RGBW_BRIGHTNESS_ONOFF 0x1
#define RGBW_COLOR_ONOFF 0x2

typedef struct {
  char brightness;
  char colorBrightness;
  char B;
  char G;
  char R;
  char onOff;
} TRGBW_Value;  // v. >= 10

#define SUPLA_RELAY_FLAG_OVERCURRENT_RELAY_OFF 0x1

typedef struct {
  char hi;  // actual state of relay  - 0 turned off, >= 1 - turned on
  unsigned short flags;  // SUPLA_RELAY_FLAG_*
} TRelayChannel_Value;   // v. >= 15

#define DIGIGLASS_TOO_LONG_OPERATION_WARNING 0x1
#define DIGIGLASS_PLANNED_REGENERATION_IN_PROGRESS 0x2
#define DIGIGLASS_REGENERATION_AFTER_20H_IN_PROGRESS 0x4

typedef struct {
  unsigned char flags;
  unsigned char sectionCount;  // 1 - 16 Filled by server
  unsigned short mask;         // bit mask. 0 - opaque, 1 - transparent
} TDigiglass_Value;            // v. >= 14

typedef struct {
  unsigned short mask;  // Bit mask. 0 - opaque, 1 - transparent
  unsigned short
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

typedef struct {
  TThermostatValueGroup Group[4];
} TThermostat_ScheduleCfg;  // v. >= 11

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

typedef struct {
  _supla_int16_t Index;  // BIT0 Temperature[0], BIT1 Temperature[1] etc...
  unsigned _supla_int16_t Temperature[10];
} TThermostatTemperatureCfg;

// Thermostat configuration commands - ver. >= 11
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

// Thermostat capability flags - ver. >= 11
#define SUPLA_THERMOSTAT_CAP_FLAG_MODE_ONOFF 0x0001
#define SUPLA_THERMOSTAT_CAP_FLAG_MODE_AUTO 0x0002
#define SUPLA_THERMOSTAT_CAP_FLAG_MODE_COOL 0x0004
#define SUPLA_THERMOSTAT_CAP_FLAG_MODE_HEAT 0x0008
#define SUPLA_THERMOSTAT_CAP_FLAG_MODE_ECO 0x0010
#define SUPLA_THERMOSTAT_CAP_FLAG_MODE_DRY 0x0020
#define SUPLA_THERMOSTAT_CAP_FLAG_MODE_FANONLY 0x0040
#define SUPLA_THERMOSTAT_CAP_FLAG_MODE_PURIFIER 0x0080
#define SUPLA_THERMOSTAT_CAP_FLAG_SCHEDULE 0x0100

// Thermostat value flags - ver. >= 11
#define SUPLA_THERMOSTAT_VALUE_FLAG_ON 0x0001
#define SUPLA_THERMOSTAT_VALUE_FLAG_AUTO_MODE 0x0002
#define SUPLA_THERMOSTAT_VALUE_FLAG_COOL_MODE 0x0004
#define SUPLA_THERMOSTAT_VALUE_FLAG_HEAT_MODE 0x0008
#define SUPLA_THERMOSTAT_VALUE_FLAG_ECO_MODE 0x0010
#define SUPLA_THERMOSTAT_VALUE_FLAG_DRY_MODE 0x0020
#define SUPLA_THERMOSTAT_VALUE_FLAG_FANONLY_MODE 0x0040
#define SUPLA_THERMOSTAT_VALUE_FLAG_PURIFIER_MODE 0x0080

// Thermostat fields - ver. >= 11
#define THERMOSTAT_FIELD_MeasuredTemperatures 0x01
#define THERMOSTAT_FIELD_PresetTemperatures 0x02
#define THERMOSTAT_FIELD_Flags 0x04
#define THERMOSTAT_FIELD_Values 0x08
#define THERMOSTAT_FIELD_Time 0x10
#define THERMOSTAT_FIELD_Schedule 0x20

typedef struct {
  unsigned char Fields;
  _supla_int16_t MeasuredTemperature[10];  // * 0.01
  _supla_int16_t PresetTemperature[10];    // * 0.01
  _supla_int16_t Flags[8];
  _supla_int16_t Values[8];
  TThermostat_Time Time;
  TThermostat_Schedule Schedule;  // 7 days x 24h (4bit/hour)
} TThermostat_ExtendedValue;      // v. >= 11

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
#define SUPLA_CHANNELSTATE_FIELD_LIGHTSOURCELIFESPAN 0x1000
#define SUPLA_CHANNELSTATE_FIELD_LIGHTSOURCEOPERATINGTIME 0x2000

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
  _supla_int_t Fields;
  _supla_int_t defaultIconField;  // SUPLA_CHANNELSTATE_FIELD_*
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
  unsigned short LightSourceLifespan;      // 0 - 65535 hours
  union {
    short LightSourceLifespanLeft;  // -327,67 - 100.00% LightSourceLifespan *
                                    // 0.01
    _supla_int_t LightSourceOperatingTime;  // -3932100sec. - 3932100sec.
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
  _supla_int_t ID;
  unsigned _supla_int_t
      CaptionSize;  // including the terminating null byte ('\0')
  char Caption[SUPLA_CAPTION_MAXSIZE];  // Last variable in struct!
} TCS_SetCaption;                       // v. >= 12

typedef struct {
  _supla_int_t ID;
  unsigned char ResultCode;
  unsigned _supla_int_t
      CaptionSize;  // including the terminating null byte ('\0')
  char Caption[SUPLA_CAPTION_MAXSIZE];  // Last variable in struct!
} TSC_SetCaptionResult;                 // v. >= 12

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
  int DeviceID;
} TCS_DeviceReconnectRequest;  // v. >= 12

typedef struct {
  int DeviceID;
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

#define SUPLA_CHANNEL_CONFIG_MAXSIZE 128
#define SUPLA_CONFIG_TYPE_DEFAULT 0

typedef struct {
  unsigned char ChannelNumber;
  unsigned char ConfigType;
  unsigned _supla_int_t Flags;
} TDS_GetChannelConfigRequest;  // v. >= 16

typedef struct {
  unsigned char ChannelNumber;
  _supla_int_t Func;
  unsigned char ConfigType;
  unsigned short ConfigSize;
  char Config[SUPLA_CHANNEL_CONFIG_MAXSIZE];  // Last variable in struct! v. >=
                                              // 16. TSD_DeviceChannelConfig_*
} TSD_ChannelConfig;

typedef struct {
  _supla_int_t TimeMS;
} TSD_ChannelConfig_StaircaseTimer;  // v. >= 16

typedef struct {
  _supla_int_t ClosingTimeMS;
  _supla_int_t OpeningTimeMS;
} TSD_ChannelConfig_Rollershutter;  // v. >= 16

typedef struct {
  unsigned _supla_int_t ActiveActions;
} TSD_ChannelConfig_ActionTrigger;  // v. >= 16

typedef struct {
  _supla_int_t ChannelID;
  unsigned _supla_int_t DurationMS;
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
                                        unsigned _supla_int_t call_type);
TSuplaDataPacket *PROTO_ICACHE_FLASH sproto_sdp_malloc(void *spd_ptr);
void PROTO_ICACHE_FLASH sproto_sdp_free(TSuplaDataPacket *sdp);

void PROTO_ICACHE_FLASH sproto_log_summary(void *spd_ptr);
void PROTO_ICACHE_FLASH sproto_buffer_dump(void *spd_ptr, unsigned char in);

#ifdef __cplusplus
}
#endif

#endif /* supla_proto_H_ */
