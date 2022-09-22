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

#ifndef SUPLA_CH_ABSTRACT_REGISTER_DEVICE_H_
#define SUPLA_CH_ABSTRACT_REGISTER_DEVICE_H_

#include <memory>

#include "conn/call_handler/abstract_register_object.h"
#include "device/abstract_device_dao.h"
#include "proto.h"

class supla_user;
class supla_device;
class supla_abstract_register_device
    : protected supla_abstract_register_object {
 private:
  std::weak_ptr<supla_device> device;
  unsigned char channel_count;
  TDS_SuplaRegisterDevice_C *register_device_c;
  TDS_SuplaRegisterDevice_E *register_device_e;
  int location_id;
  int device_flags;
  short manufacturer_id;
  short product_id;
  int device_id;

  bool location_enabled;
  bool new_device;
  bool device_enabled;
  bool channel_added;
  int _location_id;
  int _original_location_id;

  supla_abstract_device_dao *device_dao;

  void send_result(int resultcode);
  bool device_auth(void);
  bool add_device(void);
  bool add_channels(void);

 protected:
  void register_device(std::weak_ptr<supla_device> device,
                       TDS_SuplaRegisterDevice_C *register_device_c,
                       TDS_SuplaRegisterDevice_E *register_device_e,
                       supla_abstract_srpc_adapter *srpc_adapter,
                       supla_abstract_db_access_provider *dba,
                       supla_abstract_connection_dao *conn_dao,
                       supla_abstract_device_dao *device_dao, int client_sd,
                       int client_ipv4, unsigned char activity_timeout);

  virtual bool is_prev_entering_cfg_mode(void) = 0;
  virtual void on_registraction_success(void) = 0;

  supla_abstract_device_dao *get_device_dao(void);
  std::weak_ptr<supla_device> get_device(void);
  bool is_channel_added(void);
  int get_device_id(void);
  int get_device_flags(void);
  int get_location_id(void);
  int get_channel_count(void);
  TDS_SuplaDeviceChannel_B *get_channels_b(void);
  TDS_SuplaDeviceChannel_C *get_channels_c(void);
  void set_hold_time_on_failure_usec(__useconds_t hold_time_on_failure_usec);

 public:
  supla_abstract_register_device(void);
  virtual ~supla_abstract_register_device();
  __useconds_t get_hold_time_on_failure_usec(void);
};

#endif /* SUPLA_CH_ABSTRACT_REGISTER_DEVICE_H_*/
