/*
 * device.h
 *
 *  Created on: 15 lut 2020
 *      Author: beku
 */

#ifndef SRC_DEVICE_H_
#define SRC_DEVICE_H_

#include "../supla-client-lib/proto.h"
#include "gpio.h"

#define MAX_MQTT_TOPIC_LENGTH 255
#define MAX_MQTT_TEMPLATE_LENGTH 1024

typedef void (*_func_channelio_valuechanged)(
    unsigned char number, char value[SUPLA_CHANNELVALUE_SIZE], void *user_data);

typedef struct {
#ifdef __SINGLE_THREAD
  struct timeval close_time;
#endif

  struct timeval last_tv;
  char value;
  void *lck;
} TChannelGpioPortValue;

typedef struct {
  struct timeval last_tv;
  double temp;
  double humidity;
  void *lck;
} TChannelW1TempValue;

typedef struct {
  int color;
  char color_brightness;
  char brightness;
  void *lck;
} TChannelRGBWValue;

typedef struct {
#ifdef __SINGLE_THREAD
  struct timeval close_time;
#endif
  struct timeval last_tv;
  char addr;
  char reset;
  char dir;
  char port;
  char active;
  void *lck;
  TChannelGpioPortValue gpio;
} TChannelMCP23008;

typedef struct {
  char topic_in[MAX_MQTT_TOPIC_LENGTH];
  char topic_out[MAX_MQTT_TOPIC_LENGTH];
  char template_in[MAX_MQTT_TEMPLATE_LENGTH];
  char template_out[MAX_MQTT_TEMPLATE_LENGTH];
  unsigned char retain;
  char value[SUPLA_CHANNELVALUE_SIZE];
  void *lck;
} TChannelMQTTConfig;

typedef struct {
  unsigned char number;
  int type;
  int driver;
  unsigned char gpio1;
  unsigned char gpio2;
  unsigned char bistable;
  char *w1;

  TChannelGpioPortValue gpio1_value;
  TChannelGpioPortValue gpio2_value;
  TChannelW1TempValue w1_value;
  TChannelRGBWValue rgbw_value;
  TChannelMCP23008 mcp23008;
  TChannelMQTTConfig mqtt_config;

} TDeviceChannel;

typedef struct {
  TDeviceChannel *channel;
  void *sthread;
  unsigned char port1;
  unsigned char port2;
  unsigned char bistable;
  char hi;
  int time_ms;
  char raise;
} TGpioThreadItem;

typedef struct {
  char initialized;

  TDeviceChannel *channels;
  int channel_count;

  void *cb_lck;
  _func_channelio_valuechanged on_valuechanged;
  void *on_valuechanged_user_data;

  void *wl_lck;
  TGpioPort *watch_list;
  int watch_list_count;

#ifndef __SINGLE_THREAD
  void *gpio_thread_arr;
  void *w1_sthread;
  void *mcp_sthread;
#endif
} TChannelIo;

#endif /* SRC_DEVICE_H_ */
