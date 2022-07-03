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

#ifndef ABSTRACT_SRPC_ADAPTER_H_
#define ABSTRACT_SRPC_ADAPTER_H_

#include "proto.h"

class supla_abstract_srpc_adapter {
 private:
  void *srpc;

 protected:
  void *get_srpc(void);

 public:
  explicit supla_abstract_srpc_adapter(void *srpc);
  virtual ~supla_abstract_srpc_adapter();

  virtual char get_proto_version(void) = 0;

  virtual _supla_int_t sc_async_scene_pack_update(
      TSC_SuplaScenePack *scene_pack) = 0;  // ver. >= 18
  virtual _supla_int_t sc_async_scene_state_pack_update(
      TSC_SuplaSceneStatePack *scene_state_pack) = 0;  // ver. >= 18
};

#endif /* ABSTRACT_SRPC_ADAPTER_H_ */