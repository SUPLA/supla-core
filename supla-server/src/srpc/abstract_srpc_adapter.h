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

 public:
  explicit supla_abstract_srpc_adapter(void *srpc);
  virtual ~supla_abstract_srpc_adapter();

  void *get_srpc(
      void);  // TODO(przemyslawzygmunt): Move to the protected access specifier

  virtual void set_proto_version(unsigned char version) = 0;
  virtual unsigned char get_proto_version(void) = 0;

  virtual _supla_int_t sdc_async_ping_server_result(void) = 0;

  virtual _supla_int_t dcs_async_set_activity_timeout_result(
      TSDC_SuplaSetActivityTimeoutResult *sdc_set_activity_timeout_result) = 0;

  virtual _supla_int_t sdc_async_get_registration_enabled_result(
      TSDC_RegistrationEnabled *reg_enabled) = 0;

  virtual _supla_int_t sc_async_scene_pack_update(
      TSC_SuplaScenePack *scene_pack) = 0;  // ver. >= 18
  virtual _supla_int_t sc_async_scene_state_pack_update(
      TSC_SuplaSceneStatePack *scene_state_pack) = 0;  // ver. >= 18

  virtual _supla_int_t sd_async_get_firmware_update_url_result(
      TSD_FirmwareUpdate_UrlResult *result) = 0;
};

#endif /* ABSTRACT_SRPC_ADAPTER_H_ */
