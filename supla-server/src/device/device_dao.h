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

#ifndef SUPLA_DEVICE_DAO_H_
#define SUPLA_DEVICE_DAO_H_

#include <string>
#include <vector>

#include "db/abstract_db_access_provider.h"
#include "device/abstract_device_dao.h"

class supla_device_dao : public supla_abstract_device_dao {
 private:
  supla_abstract_db_access_provider *dba;

 public:
  explicit supla_device_dao(supla_abstract_db_access_provider *dba);
  virtual ~supla_device_dao();

  virtual bool get_device_firmware_update_url(
      int device_id, TDS_FirmwareUpdateParams *params,
      TSD_FirmwareUpdate_UrlResult *url);

  virtual bool location_auth(int location_id, char *location_pwd, int *user_id,
                             bool *is_enabled);

  virtual bool get_authkey_hash(int id, char authkey_hash[BCRYPT_HASH_MAXSIZE],
                                bool *is_null);

  virtual int get_location_id(int user_id, bool enabled);

  virtual bool get_device_id(int user_id, const char guid[SUPLA_GUID_SIZE],
                             int *id);

  virtual int get_device_id(int user_id, const char guid[SUPLA_GUID_SIZE]);

  virtual bool get_device_reg_enabled(int user_id);

  virtual int get_device_limit_left(int user_id);

  virtual bool get_device_variables(int device_id, bool *device_enabled,
                                    int *original_location_id, int *location_id,
                                    bool *location_enabled);

  virtual int get_channel_id_and_type(int device_id, int channel_number,
                                      int *type);

  virtual int get_device_channel_count(int device_id);

  virtual int add_device(int location_id, const char guid[SUPLA_GUID_SIZE],
                         const char *authkey, const char *name,
                         unsigned int ipv4, const char *softver,
                         int proto_version, short manufacturer_id,
                         short product_id, int flags, int user_id);

  virtual bool update_device(int device_id, int original_location_id,
                             const char *authkey, const char *name,
                             unsigned int ipv4, const char *softver,
                             int proto_version, int flags);

  virtual int add_channel(int device_id, int channel_number, int type, int func,
                          int param1, int param2, int flist, int flags,
                          int user_id);

  virtual bool on_new_device(int device_id);

  virtual bool on_channel_added(int device_id, int channel_id);

  virtual bool set_device_config(int user_id, int device_id,
                                 device_json_config *config,
                                 unsigned _supla_int16_t available_fields);

  virtual device_json_config *get_device_config(int device_id,
                                                std::string *md5sum);

  virtual std::vector<supla_device_channel *> get_channels(
      supla_device *device);

  virtual void set_channel_properties(int user_id, int channel_id,
                                      channel_json_config *config);

  virtual bool set_channel_user_config(int user_id, int channel_id,
                                       channel_json_config *config);

  virtual void update_channel_value(int channel_id, int user_id,
                                    const char value[SUPLA_CHANNELVALUE_SIZE],
                                    unsigned _supla_int_t validity_time_sec);

  virtual void update_channel_extended_value(int channel_id, int user_id,
                                             supla_channel_extended_value *ev);
  virtual channel_json_config *get_channel_config(
      int channel_id, std::string *user_config_md5sum,
      std::string *properties_md5sum);
};

#endif /* SUPLA_DEVICE_DAO_H_ */
