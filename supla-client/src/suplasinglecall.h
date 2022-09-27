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

#ifndef SRC_SUPLASINGLECALL_H_
#define SRC_SUPLASINGLECALL_H_

#include <functional>
#include <list>

#include "eh.h"
#include "proto.h"
#include "srpc.h"

class supla_single_call {
 private:
  TCS_ClientAuthorizationDetails auth_details;
  int protocol_version;
  void *ssd;
  void *srpc;
  TEventHandler *eh;
  std::list<TsrpcReceivedData> received_data;
  unsigned char min_version_required;

  static int socket_read(void *buf, int count, void *single_call);
  static int socket_write(void *buf, int count, void *single_call);
  static void on_remote_call_received(void *_srpc, unsigned int rr_id,
                                      unsigned int call_id, void *single_call,
                                      unsigned char proto_version);
  static void on_min_version_required(void *_srpc,
                                      unsigned _supla_int_t call_id,
                                      unsigned char min_version,
                                      void *single_call);

  int socket_read(void *buf, int count);
  int socket_write(void *buf, int count);
  void on_remote_call_received(unsigned int call_id);
  void on_min_version_required(unsigned char min_version);

  int connect(void);
  void cleanup(void);
  int make_call(
      std::function<void(void)> call,
      std::function<bool(int *result, TsrpcReceivedData *rd)> process_response);

 public:
  supla_single_call(TCS_ClientAuthorizationDetails *auth_details,
                    int protocol_version);
  virtual ~supla_single_call();
  unsigned char get_min_version_number(void);
  int execute_action(TCS_Action *action);
};

#endif /* SRC_SUPLASINGLECALL_H_ */
