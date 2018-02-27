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

class supla_client;
class supla_device;
class cdcommon;

class serverconnection {
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
    cdcommon *cdptr;
  };

  char registered;

 public:
  serverconnection(void *ssd, void *supla_socket, unsigned int client_ipv4);
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
