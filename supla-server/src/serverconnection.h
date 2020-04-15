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

#ifndef SERVERCONNECTION_H_
#define SERVERCONNECTION_H_

#include <stddef.h>
#include <sys/time.h>
#include "eh.h"
#include "srpc.h"

#define LOCAL_IPV4_ARRAY_SIZE 5

class supla_client;
class supla_device;
class cdbase;

class serverconnection {
 private:
  static void *reg_pending_arr;
  static void read_local_ipv4_addresses(void);
  void set_registered(char registered);
  void on_device_reconnect_request(
      void *_srpc, TCS_DeviceReconnectRequest *cs_device_reconnect_request);
  void on_set_channel_function_request(
      TCS_SetChannelFunction *cs_set_channel_function);
  void on_set_channel_caption_request(
      TCS_SetChannelCaption *cs_set_channel_caption);
  void on_register_device_request(void *_srpc, unsigned int call_type,
                                  unsigned char proto_version,
                                  TsrpcReceivedData *rd);

 protected:
  unsigned int client_ipv4;
  void *ssd;
  void *supla_socket;
  void *_srpc;
  void *sthread;
  TEventHandler *eh;

  struct timeval init_time;
  unsigned char activity_timeout;

  union {
    supla_client *client;
    supla_device *device;
    cdbase *cdptr;
  };

  char registered;

  int incorrect_call_counter;
  void catch_incorrect_call(unsigned int call_type);

 public:
  static unsigned int local_ipv4[LOCAL_IPV4_ARRAY_SIZE];
  serverconnection(void *ssd, void *supla_socket, unsigned int client_ipv4);
  static void init(void);
  static void serverconnection_free(void);
  static int registration_pending_count();
  void execute(void *sthread);
  void terminate(void);
  virtual ~serverconnection();

  int socket_read(void *buf, size_t count);
  int socket_write(const void *buf, size_t count);
  void on_remote_call_received(void *_srpc, unsigned int rr_id,
                               unsigned int call_type,
                               unsigned char proto_version);
  void *srpc(void);
  unsigned int getClientIpv4(void);
  int getClientSD(void);
  unsigned char GetActivityTimeout(void);
  unsigned char getProtocolVersion(void);
};

#endif /* SERVERCONNECTION_H_ */
