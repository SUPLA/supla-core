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

#ifndef DEVICE_DAO_MOCK_H_
#define DEVICE_DAO_MOCK_H_

#include <gmock/gmock.h>

#include <string>
#include <vector>

#include "device/abstract_device_dao.h"

namespace testing {

class DeviceDaoMock : public supla_abstract_device_dao {
 public:
  DeviceDaoMock(void);
  virtual ~DeviceDaoMock(void);

  MOCK_METHOD3(get_device_firmware_update_url,
               bool(int device_id, TDS_FirmwareUpdateParams *params,
                    TSD_FirmwareUpdate_UrlResult *url));

  MOCK_METHOD4(location_auth, bool(int location_id, char *location_pwd,
                                   int *user_id, bool *is_enabled));

  MOCK_METHOD2(get_location_id, int(int user_id, bool enabled));

  MOCK_METHOD3(get_authkey_hash,
               bool(int id, char authkey_hash[BCRYPT_HASH_MAXSIZE],
                    bool *is_null));

  MOCK_METHOD3(get_device_id,
               bool(int user_id, const char guid[SUPLA_GUID_SIZE], int *id));

  MOCK_METHOD2(get_device_id,
               int(int user_id, const char guid[SUPLA_GUID_SIZE]));

  MOCK_METHOD1(get_device_reg_enabled, bool(int user_id));

  MOCK_METHOD1(get_device_limit_left, int(int user_id));

  MOCK_METHOD7(get_device_variables,
               bool(int device_id, bool *device_enabled,
                    int *original_location_id, int *location_id,
                    bool *location_enabled, int *flags,
                    bool *channel_addition_blocked));

  MOCK_METHOD4(get_channel_properties,
               int(int device_id, int channel_number, int *type, int *flist));

  MOCK_METHOD1(get_device_channel_count, int(int device_id));

  virtual int add_device(int location_id, const char guid[SUPLA_GUID_SIZE],
                         const char *authkey, const char *name,
                         unsigned int ipv4, const char *softver,
                         int proto_version, short manufacturer_id,
                         short product_id, int flags, int user_id);

  MOCK_METHOD10(add_device,
                void(int location_id, const char guid[SUPLA_GUID_SIZE],
                     const char *authkey, const char *name, unsigned int ipv4,
                     const char *softver, int proto_version,
                     short manufacturer_id, short product_id, int flags));

  MOCK_METHOD1(add_device, int(int user_id));

  MOCK_METHOD8(update_device,
               bool(int device_od, int original_location_id,
                    const char *authkey, const char *name, unsigned int ipv4,
                    const char *softver, int proto_version, int flags));

  MOCK_METHOD10(add_channel_a,
                int(int device_id, int channel_number, int type, int func,
                    int param1, int param2, int flist, _supla_int64_t flags,
                    int alt_icon, int user_id));

  MOCK_METHOD4(add_channel_b, int(int device_id, int channel_number,
                                  unsigned short sub_device_id, int user_id));

  virtual int add_channel(int device_id, int channel_number, int type, int func,
                          int param1, int param2, int flist,
                          _supla_int64_t flags, int alt_icon,
                          unsigned short sub_device_id, int user_id);

  MOCK_METHOD1(on_new_device, bool(int device_id));

  MOCK_METHOD2(on_channel_added, bool(int device_id, int channel_id));

  MOCK_METHOD4(set_device_config,
               bool(int user_id, int device_id, device_json_config *config,
                    unsigned _supla_int16_t available_fields));

  MOCK_METHOD3(get_device_config,
               device_json_config *(int device_id,
                                    std::string *user_config_md5sum,
                                    std::string *properties_md5sum));

  MOCK_METHOD1(get_channels,
               std::vector<supla_device_channel *>(supla_device *device));

  MOCK_METHOD3(set_channel_properties,
               void(int user_id, int channel_id, supla_json_config *config));

  MOCK_METHOD2(erase_channel_properties, void(int user_id, int channel_id));

  MOCK_METHOD3(get_channel_config,
               supla_json_config *(int channel_id,
                                   std::string *user_config_md5sum,
                                   std::string *properties_md5sum));

  MOCK_METHOD3(set_channel_config,
               bool(int user_id, int channel_id, supla_json_config *config));

  MOCK_METHOD4(update_channel_value,
               void(int channel_id, int user_id,
                    const char value[SUPLA_CHANNELVALUE_SIZE],
                    unsigned _supla_int_t validity_time_sec));

  MOCK_METHOD3(update_channel_extended_value,
               void(int channel_id, int user_id,
                    supla_channel_extended_value *ev));

  MOCK_METHOD3(update_channel_functions,
               void(int channel_id, int user_id, int flist));

  MOCK_METHOD1(get_channel_fragments,
               std::vector<supla_channel_fragment>(int device_id));

  MOCK_METHOD2(get_channel_fragment,
               supla_channel_fragment(int device_id, int channel_number));

  MOCK_METHOD3(update_channel_conflict_details,
               void(int device_id, int channel_number, char *details));

  MOCK_METHOD2(update_device_pairing_result,
               void(int device_id, char *pairing_result));

  MOCK_METHOD2(set_subdevice_details,
               void(int device_id, TDS_SubdeviceDetails *details));

  MOCK_METHOD2(get_channel_extended_value,
               supla_channel_extended_value *(int user_id, int channel_id));
};

} /* namespace testing */

#endif /* DEVICE_DAO_MOCK_H_ */
