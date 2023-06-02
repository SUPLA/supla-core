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

#ifndef SUPLA_CH_SET_DEVICE_CONFIG_H_
#define SUPLA_CH_SET_DEVICE_CONFIG_H_

#include <memory>

#include "device/call_handler/abstract_device_srpc_call_handler.h"
#include "device/call_handler/set_device_config_multipart_call.h"

class supla_ch_set_device_config
    : public supla_abstract_device_srpc_call_handler {
 protected:
  void handle_multipart_call(std::shared_ptr<supla_device> device,
                             supla_set_device_multipart_call* mp_call);
  virtual void handle_call(std::shared_ptr<supla_device> device,
                           supla_abstract_srpc_adapter* srpc_adapter,
                           TsrpcReceivedData* rd, unsigned int call_id,
                           unsigned char proto_version);

 public:
  supla_ch_set_device_config(void);
  virtual ~supla_ch_set_device_config();
  virtual bool can_handle_call(unsigned int call_id);
};

#endif /* SUPLA_CH_SET_DEVICE_CONFIG_H_*/
