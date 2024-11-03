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

#ifndef SUPLA_ABSTRACT_DEVICE_DAO_H_
#define SUPLA_ABSTRACT_DEVICE_DAO_H_

#include <string>
#include <vector>

#include "device/channel_fragment.h"
#include "device/extended_value/channel_extended_value.h"
#include "jsonconfig/device/device_json_config.h"
#include "jsonconfig/json_config.h"
#include "proto.h"
#include "tools.h"

class supla_device;
class supla_device_channel;
class supla_abstract_device_dao {
 public:
  supla_abstract_device_dao();
  virtual ~supla_abstract_device_dao();

  virtual bool get_device_firmware_update_url(
      int device_id, TDS_FirmwareUpdateParams *params,
      TSD_FirmwareUpdate_UrlResult *url) = 0;

  virtual bool location_auth(int location_id, char *location_pwd, int *user_id,
                             bool *is_enabled) = 0;

  virtual bool get_authkey_hash(int id, char authkey_hash[BCRYPT_HASH_MAXSIZE],
                                bool *is_null) = 0;

  virtual int get_location_id(int user_id, bool enabled) = 0;

  virtual bool get_device_id(int user_id, const char guid[SUPLA_GUID_SIZE],
                             int *id) = 0;

  virtual int get_device_id(int user_id, const char guid[SUPLA_GUID_SIZE]) = 0;

  virtual bool get_device_reg_enabled(int user_id) = 0;

  virtual int get_device_limit_left(int user_id) = 0;

  virtual bool get_device_variables(int device_id, bool *device_enabled,
                                    int *original_location_id, int *location_id,
                                    bool *location_enabled, int *flags,
                                    bool *channel_addition_blocked) = 0;

  virtual int get_channel_properties(int device_id, int channel_number,
                                     int *type, int *flist) = 0;

  virtual int get_device_channel_count(int device_id) = 0;

  virtual int add_device(int location_id, const char guid[SUPLA_GUID_SIZE],
                         const char *authkey, const char *name,
                         unsigned int ipv4, const char *softver,
                         int proto_version, short manufacturer_id,
                         short product_id, int flags, int user_id) = 0;

  virtual bool update_device(int device_od, int original_location_id,
                             const char *authkey, const char *name,
                             unsigned int ipv4, const char *softver,
                             int proto_version, int flags) = 0;

  virtual int add_channel(int device_id, int channel_number, int type, int func,
                          int param1, int param2, int flist,
                          _supla_int64_t flags, int alt_icon,
                          unsigned short sub_device_id, int user_id) = 0;

  virtual bool on_new_device(int device_id) = 0;

  virtual bool on_channel_added(int device_id, int channel_id) = 0;

  virtual bool set_device_config(int user_id, int device_id,
                                 device_json_config *config,
                                 unsigned _supla_int16_t available_fields) = 0;

  virtual device_json_config *get_device_config(
      int device_id, std::string *user_config_md5sum,
      std::string *properties_md5sum) = 0;

  // Perhaps this method should be added to the repository
  virtual std::vector<supla_device_channel *> get_channels(
      supla_device *device) = 0;

  // The following methods should be moved to channel_dao
  virtual void set_channel_properties(int user_id, int channel_id,
                                      supla_json_config *config) = 0;

  virtual void erase_channel_properties(int user_id, int channel_id) = 0;

  virtual bool set_channel_config(int user_id, int channel_id,
                                  supla_json_config *config) = 0;

  virtual void update_channel_value(
      int channel_id, int user_id, const char value[SUPLA_CHANNELVALUE_SIZE],
      unsigned _supla_int_t validity_time_sec) = 0;

  virtual void update_channel_extended_value(
      int channel_id, int user_id, supla_channel_extended_value *ev) = 0;

  virtual void update_channel_functions(int channel_id, int user_id,
                                        int flist) = 0;

  virtual supla_json_config *get_channel_config(
      int channel_id, std::string *user_config_md5sum,
      std::string *properties_md5sum) = 0;

  virtual std::vector<supla_channel_fragment> get_channel_fragments(
      int device_id) = 0;

  virtual supla_channel_fragment get_channel_fragment(int device_id,
                                                      int channel_number) = 0;

  virtual void update_channel_conflict_details(int device_id,
                                               int channel_number,
                                               char *details) = 0;

  virtual void update_device_pairing_result(int device_id,
                                            char *pairing_result) = 0;

  virtual void set_subdevice_details(int device_id,
                                     TDS_SubdeviceDetails *details) = 0;

  virtual supla_channel_extended_value *get_channel_extended_value(
      int user_id, int channel_id) = 0;
};

#endif /* SUPLA_ABSTRACT_DEVICE_DAO_H_ */
