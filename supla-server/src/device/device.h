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

#ifndef DEVICE_H_
#define DEVICE_H_

#include <conn/abstract_connection_object.h>

#include <list>
#include <memory>

#include "device/devicechannels.h"

class supla_user;
class supla_device_call_handler_collection;
class supla_register_device;
class supla_ch_device_calcfg_result;
class supla_device : public supla_abstract_connection_object {
 private:
  int flags;
  static supla_device_call_handler_collection call_handler_collection;
  supla_device_channels *channels;
  bool calcfg_cmd(unsigned _supla_int64_t flag, _supla_int_t cmd,
                  unsigned _supla_int_t data_size,
                  char data[SUPLA_CALCFG_DATA_MAXSIZE],
                  bool importat_for_sleepers);

 protected:
  friend class supla_register_device;
  friend class supla_ch_device_calcfg_result;

  int last_calcfg_command_importatnt_for_sleepers;

  void set_flags(int flags);
  void set_channels(supla_device_channels *channels);

  virtual bool can_reconnect(void);

 public:
  explicit supla_device(supla_connection *connection);
  virtual ~supla_device();

  std::shared_ptr<supla_device> get_shared_ptr(void);
  virtual bool is_sleeping_object(void);
  virtual unsigned int get_time_to_wakeup_msec(void);
  virtual supla_abstract_srpc_call_handler_collection *
  get_srpc_call_handler_collection(void);
  static bool funclist_contains_function(int funcList, int func);

  supla_device_channels *get_channels(void);
  int get_flags(void);

  bool enter_cfg_mode(void);
  void send_config_to_device(void);
  bool pair_subdevice(const supla_caller &caller, bool superuser_authorized);
  bool calcfg_identify(void);
  bool calcfg_restart(void);
  bool check_updates(void);
  bool start_update(void);
  bool factory_reset(void);
  bool set_cfg_mode_password(const char *password);
};

#endif /* DEVICE_H_ */
