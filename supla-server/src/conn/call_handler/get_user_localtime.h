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

#ifndef SUPLA_CH_GET_USER_LOCALTIME_H_
#define SUPLA_CH_GET_USER_LOCALTIME_H_

#include <memory>

#include "srpc/abstract_srpc_call_hanlder.h"

class supla_ch_get_user_localtime : public supla_abstract_srpc_call_handler {
 public:
  supla_ch_get_user_localtime(void);
  virtual ~supla_ch_get_user_localtime();
  virtual bool can_handle_call(unsigned int call_id);
  virtual void handle_call(
      std::shared_ptr<supla_abstract_connection_object> object,
      supla_abstract_srpc_adapter* srpc_adapter, TsrpcReceivedData* rd,
      unsigned int call_id, unsigned char proto_version);
};

#endif /* SUPLA_CH_GET_USER_LOCALTIME_H_*/
