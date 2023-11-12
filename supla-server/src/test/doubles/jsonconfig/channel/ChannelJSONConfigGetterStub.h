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

#ifndef supla_json_config_GETTER_STUB_H_
#define supla_json_config_GETTER_STUB_H_

#include "jsonconfig/abstract_json_config_getter.h"

namespace testing {

class ChannelJSONConfigGetterStub : public abstract_json_config_getter {
 private:
  supla_json_config *config;

 protected:
 public:
  ChannelJSONConfigGetterStub(void);
  virtual ~ChannelJSONConfigGetterStub(void);
  void set_config(supla_json_config *config);
  virtual supla_json_config *get_config(int user_id, int device_id,
                                        int channel_id);
};

} /* namespace testing */

#endif /* supla_json_config_GETTER_STUB_H_ */
