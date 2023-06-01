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

#ifndef SUPLA_CH_ABSTRACT_DEVICE_CHANNEL_VALUE_CHANED_H_
#define SUPLA_CH_ABSTRACT_DEVICE_CHANNEL_VALUE_CHANED_H_

#include <memory>

#include "device/call_handler/abstract_device_srpc_call_handler.h"

class supla_ch_abstract_device_channel_value_changed
    : public supla_abstract_device_srpc_call_handler {
 protected:
  void on_channel_value_changed(std::shared_ptr<supla_device> device,
                                unsigned char channel_number, char *value,
                                bool offline,
                                unsigned _supla_int_t *validity_time_sec);

 public:
  supla_ch_abstract_device_channel_value_changed(void);
  virtual ~supla_ch_abstract_device_channel_value_changed();
};

#endif /* SUPLA_CH_ABSTRACT_DEVICE_CHANNEL_VALUE_CHANED_H_*/
