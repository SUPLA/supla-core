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

#include "device/calcfg_queue.h"
#include "device/devicechannels.h"

class supla_user;
class supla_device_call_handler_collection;
class supla_register_device;
class supla_device : public supla_abstract_connection_object {
 private:
  int flags;
  short manufacturer_id;
  static supla_device_call_handler_collection call_handler_collection;
  supla_device_channels *channels;
  supla_device_calcfg_queue calcfg_queue;
  bool calcfg_cmd(unsigned _supla_int64_t flag, _supla_int_t cmd,
                  unsigned _supla_int_t data_size,
                  char data[SUPLA_CALCFG_DATA_MAXSIZE],
                  unsigned _supla_int64_t *queued_at = nullptr,
                  bool *waiting_for_result = nullptr);

 protected:
  friend class supla_register_device;

  void set_flags(int flags);
  void set_manufacturer_id(short manufacturer_id);
  void set_channels(supla_device_channels *channels);

  virtual bool can_reconnect(void);

 public:
  explicit supla_device(supla_connection *connection);
  virtual ~supla_device();

  std::shared_ptr<supla_device> get_shared_ptr(void);
  virtual bool is_sleeping_object(void);
  virtual unsigned int get_time_to_wakeup_msec(void);
  virtual void iterate(void);
  virtual unsigned _supla_int64_t wait_time_usec(void);
  virtual supla_abstract_srpc_call_handler_collection *
  get_srpc_call_handler_collection(void);
  static bool funclist_contains_function(int funcList, int func);

  supla_device_channels *get_channels(void);
  int get_flags(void);
  short get_manufacturer_id(void);

  bool enter_cfg_mode(unsigned _supla_int64_t *queued_at = nullptr,
                      bool *waiting_for_result = nullptr);
  void send_config_to_device(void);
  void send_sync_done_to_device(void);
  bool send_calcfg_request(TSD_DeviceCalCfgRequest *request,
                           unsigned _supla_int64_t *queued_at = nullptr,
                           bool *waiting_for_result = nullptr);
  void send_queued_calcfg_requests(void);
  void on_calcfg_result(TDS_DeviceCalCfgResult *result);
  int take_latest_calcfg_command(void);
  size_t get_calcfg_queue_size(void);
  bool pair_subdevice(const supla_caller &caller, bool superuser_authorized,
                      unsigned _supla_int64_t *queued_at = nullptr,
                      bool *waiting_for_result = nullptr);
  bool calcfg_identify(unsigned _supla_int64_t *queued_at = nullptr,
                       bool *waiting_for_result = nullptr);
  bool calcfg_restart(unsigned _supla_int64_t *queued_at = nullptr,
                      bool *waiting_for_result = nullptr);
  bool check_updates(unsigned _supla_int64_t *queued_at = nullptr,
                     bool *waiting_for_result = nullptr);
  bool start_update(unsigned _supla_int64_t *queued_at = nullptr,
                    bool *waiting_for_result = nullptr);
  bool factory_reset(unsigned _supla_int64_t *queued_at = nullptr,
                     bool *waiting_for_result = nullptr);
  bool set_cfg_mode_password(const char *password,
                             unsigned _supla_int64_t *queued_at = nullptr,
                             bool *waiting_for_result = nullptr);
};

#endif /* DEVICE_H_ */
