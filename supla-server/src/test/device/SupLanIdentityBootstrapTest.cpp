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

#include <cstdio>
#include <memory>

#include "device/RegisterDeviceEssentialTest.h"
#include "device/call_handler/register_device.h"
#include "device/device.h"
#include "srpc/abstract_srpc_call_hanlder_collection.h"
#include "suplan/identity_bootstrap.h"

namespace testing {

class SupLanTestDevice : public supla_device {
 public:
  SupLanTestDevice() : supla_device(nullptr) {
    set_flags(SUPLA_DEVICE_FLAG_SUPLAN_SUPPORTED |
              SUPLA_DEVICE_FLAG_SYNC_DONE_SUPPORTED);
    set_registered(true);
  }
};

// Exercise the production registration-sync entry point and result dispatcher;
// only the existing configuration sender is replaced to avoid a real socket/DB.
class SupLanSyncTestDevice : public SupLanTestDevice {
 public:
  MOCK_METHOD(void, send_registration_config, (), (override));
  void registration_flags(int flags) { set_flags(flags); }
};

// Reuse the established authentication/DAO fixture while exercising the
// concrete after_registration_success hook rather than replacing it.
class SupLanHookRegistration : public supla_register_device {
 private:
  RegisterDeviceMock *mock;

 protected:
  supla_authkey_cache *get_authkey_cache() override {
    return mock->get_authkey_cache();
  }
  int get_user_id_by_email(const char email[SUPLA_EMAIL_MAXSIZE]) override {
    return mock->get_user_id_by_email(email);
  }
  bool get_object_id(int user_id, const char guid[SUPLA_GUID_SIZE],
                     int *id) override {
    return mock->get_object_id(user_id, guid, id);
  }
  bool get_authkey_hash(int id, char hash[BCRYPT_HASH_MAXSIZE],
                        bool *is_null) override {
    return mock->get_authkey_hash(id, hash, is_null);
  }
  void on_registration_success() override { mock->on_registration_success(); }

 public:
  explicit SupLanHookRegistration(RegisterDeviceMock *mock) : mock(mock) {}
  void run(std::shared_ptr<supla_device> device,
           TDS_SuplaRegisterDevice_G *registration,
           supla_abstract_srpc_adapter *srpc,
           supla_mariadb_access_provider *dba,
           supla_abstract_device_dao *dao) {
    supla_abstract_register_device::register_device(
        device, nullptr, registration, srpc, dba, nullptr, dao, 169, 4567, 20);
  }
};

class SupLanRegistrationTest : public RegisterDeviceEssentialTest {
 protected:
  void register_with_identities(int mode, bool production_hook = false);
};

void SupLanRegistrationTest::register_with_identities(int mode,
                                                     bool production_hook) {
  int count = mode == 3 ? SUPLA_CHANNELMAXCOUNT : 3;
  int flags = mode == 0 ? 0 : SUPLA_DEVICE_FLAG_SUPLAN_SUPPORTED;
  if (mode != 4) flags |= SUPLA_DEVICE_FLAG_SYNC_DONE_SUPPORTED;
  int version = mode == 1 ? 28 : 29;
  TDS_SuplaRegisterDevice_G registration = {};
  registration.GUID[0] = 1;
  registration.AuthKey[0] = 2;
  registration.Flags = flags;
  registration.channel_count = count;
  snprintf(registration.Email, SUPLA_EMAIL_MAXSIZE, "test@example.org");
  ON_CALL(srpcAdapter, get_proto_version).WillByDefault(Return(version));
  ON_CALL(dao, get_device_id(_, _)).WillByDefault(Return(7654));
  ON_CALL(dao, get_device_variables(_, _, _, _, _, _, _))
      .WillByDefault(
          [](int, bool *, int *, int *location, bool *enabled, int *, bool *) {
            *location = 155;
            *enabled = true;
            return true;
          });
  ON_CALL(dao, update_device).WillByDefault(Return(true));
  ON_CALL(dao, get_device_channel_count).WillByDefault(Return(count));
  for (int i = 0; i < count; i++) {
    // Deliberately reversed, non-contiguous registration numbers.
    int number = 255 - i * 2;
    registration.channels[i].Number = number;
    registration.channels[i].Type = SUPLA_CHANNELTYPE_RELAY;
    EXPECT_CALL(dao, get_channel_properties(7654, number, _, _))
        .WillOnce([i](int, int, int *type, int *functions) {
          *type = i % 2 ? SUPLA_CHANNELTYPE_RELAY : 0;
          *functions = 0;
          return i % 2 ? 90000 + i : 0;
        });
    if (!(i % 2)) {
      EXPECT_CALL(dao, add_channel_a(7654, number, _, _, _, _, _, _, _, _))
          .WillOnce(Return(90000 + i));
    }
  }
  supla_suplan_identity_bootstrap bootstrap;
  bool capable = version >= 29 &&
                 (flags & SUPLA_DEVICE_FLAG_SUPLAN_SUPPORTED) &&
                 (flags & SUPLA_DEVICE_FLAG_SYNC_DONE_SUPPORTED);
  InSequence sequence;
  EXPECT_CALL(rd, on_registration_success).Times(1);
  EXPECT_CALL(srpcAdapter, sd_async_registerdevice_result(_))
      .WillOnce([](TSD_SuplaRegisterDeviceResult *r) {
        EXPECT_EQ(SUPLA_RESULTCODE_TRUE, r->result_code);
        return 1;
      });
  if (!production_hook) {
    EXPECT_CALL(rd, after_registration_success).WillOnce([&]() {
      bootstrap.start(&srpcAdapter, flags, rd.get_device_identities());
    });
  }
  if (capable) {
    EXPECT_CALL(srpcAdapter, sd_async_suplan_device_identities(_))
        .WillOnce([count](TSD_SuplaDeviceIdentities *ids) {
          EXPECT_EQ(7654, ids->DeviceId);
          EXPECT_EQ(count, ids->ChannelCount);
          for (int i = 0; i < count; i++) {
            EXPECT_EQ(90000 + i, ids->ChannelId[i]);
          }
          return 1;
        });
  } else {
    EXPECT_CALL(srpcAdapter, sd_async_suplan_device_identities(_)).Times(0);
  }
  if (production_hook) {
    auto device = std::make_shared<SupLanSyncTestDevice>();
    device->registration_flags(flags);
    EXPECT_CALL(*device, send_registration_config()).Times(capable ? 0 : 1);
    SupLanHookRegistration hook(&rd);
    hook.run(device, &registration, &srpcAdapter, &dba, &dao);
    EXPECT_EQ(0u, device->get_suplan_root_epoch());
    Mock::VerifyAndClearExpectations(device.get());
    if (capable) {
      EXPECT_CALL(*device, send_registration_config()).WillOnce([&]() {
        EXPECT_EQ(123u, device->get_suplan_root_epoch());
      });
      TDS_SuplaDeviceIdentitiesResult result = {SUPLA_SUPLAN_RESULT_OK, 123u};
      device->on_suplan_device_identities_result(&result);
    }
  } else {
    rd.register_device(nullptr, &registration, &srpcAdapter, &dba, &dao, 169,
                       4567, 20);
    EXPECT_EQ(capable, bootstrap.is_pending());
  }
  EXPECT_FALSE(bootstrap.is_ready());
  EXPECT_EQ(0u, bootstrap.get_root_epoch());
}

TEST_F(SupLanRegistrationTest, protocol29WithoutFlagKeepsOrdinaryRegistration) {
  register_with_identities(0);
}

TEST_F(SupLanRegistrationTest, flagWithProtocol28DoesNotBootstrap) {
  register_with_identities(1);
}

TEST_F(SupLanRegistrationTest, sendsActualIdsInReorderedRegistrationSequence) {
  register_with_identities(2);
}

TEST_F(SupLanRegistrationTest, supLanWithoutSyncDoneKeepsOrdinaryRegistration) {
  register_with_identities(4);
}

TEST_F(SupLanRegistrationTest, concreteRegistrationHookWaitsForOk) {
  register_with_identities(2, true);
}

TEST_F(SupLanRegistrationTest, concreteLegacyHookContinuesWithoutIdentityAck) {
  register_with_identities(0, true);
}

TEST_F(SupLanRegistrationTest, maximumChannelCount) {
  register_with_identities(3);
}

class SupLanIdentityBootstrapTest : public Test {
 protected:
  SrpcAdapterMock srpc;
  supla_suplan_identity_bootstrap bootstrap;
  TSD_SuplaDeviceIdentities ids = {};
  void SetUp() override {
    ids.DeviceId = 7654;
    ids.ChannelCount = 2;
    ids.ChannelId[0] = 991;
    ids.ChannelId[1] = 17;
    ON_CALL(srpc, get_proto_version).WillByDefault(Return(29));
    ON_CALL(srpc, sd_async_suplan_device_identities(_))
        .WillByDefault(Return(1));
  }
  void start() {
    bootstrap.start(&srpc, SUPLA_DEVICE_FLAG_SUPLAN_SUPPORTED |
                               SUPLA_DEVICE_FLAG_SYNC_DONE_SUPPORTED,
                    &ids);
  }
};

TEST_F(SupLanIdentityBootstrapTest, readyOnlyAfterOkWithRootEpoch) {
  start();
  EXPECT_TRUE(bootstrap.is_pending());
  EXPECT_FALSE(bootstrap.is_ready());
  TDS_SuplaDeviceIdentitiesResult result = {SUPLA_SUPLAN_RESULT_OK, 123456u};
  bootstrap.on_result(&result);
  EXPECT_FALSE(bootstrap.is_pending());
  EXPECT_TRUE(bootstrap.is_ready());
  EXPECT_EQ(123456u, bootstrap.get_root_epoch());
}

TEST_F(SupLanIdentityBootstrapTest, nonOkAndLateOkFailClosed) {
  start();
  TDS_SuplaDeviceIdentitiesResult result = {
      SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR, 123u};
  bootstrap.on_result(&result);
  EXPECT_FALSE(bootstrap.is_ready());
  result.Result = SUPLA_SUPLAN_RESULT_OK;
  bootstrap.on_result(&result);
  EXPECT_FALSE(bootstrap.is_ready());
}

TEST_F(SupLanIdentityBootstrapTest, malformedAndInvalidEpochFailClosed) {
  start();
  bootstrap.on_result(nullptr);
  EXPECT_FALSE(bootstrap.is_pending());
  EXPECT_FALSE(bootstrap.is_ready());
  for (unsigned epoch : {0u, 0xFFFFFFFFu}) {
    start();
    TDS_SuplaDeviceIdentitiesResult result = {SUPLA_SUPLAN_RESULT_OK, epoch};
    bootstrap.on_result(&result);
    EXPECT_FALSE(bootstrap.is_ready());
  }
}

TEST_F(SupLanIdentityBootstrapTest, unsolicitedAndDuplicateResponseFailClosed) {
  TDS_SuplaDeviceIdentitiesResult result = {SUPLA_SUPLAN_RESULT_OK, 5u};
  bootstrap.on_result(&result);
  EXPECT_FALSE(bootstrap.is_ready());
  start();
  bootstrap.on_result(&result);
  ASSERT_TRUE(bootstrap.is_ready());
  bootstrap.on_result(&result);
  EXPECT_FALSE(bootstrap.is_ready());
}

TEST_F(SupLanIdentityBootstrapTest, disconnectAbandonsPendingContext) {
  start();
  bootstrap.reset();
  TDS_SuplaDeviceIdentitiesResult result = {SUPLA_SUPLAN_RESULT_OK, 5u};
  bootstrap.on_result(&result);
  EXPECT_FALSE(bootstrap.is_ready());
  EXPECT_EQ(0u, bootstrap.get_root_epoch());
}

TEST_F(SupLanIdentityBootstrapTest, newRegistrationInvalidatesReadyOrPending) {
  start();
  TDS_SuplaDeviceIdentitiesResult result = {SUPLA_SUPLAN_RESULT_OK, 5u};
  bootstrap.on_result(&result);
  ASSERT_TRUE(bootstrap.is_ready());
  start();
  EXPECT_TRUE(bootstrap.is_pending());
  EXPECT_FALSE(bootstrap.is_ready());
  bootstrap.start(&srpc, 0, &ids);
  EXPECT_FALSE(bootstrap.is_pending());
  bootstrap.on_result(&result);
  EXPECT_FALSE(bootstrap.is_ready());
}

TEST_F(SupLanIdentityBootstrapTest, failedSendDoesNotAllowResult) {
  EXPECT_CALL(srpc, sd_async_suplan_device_identities(_)).WillOnce(Return(0));
  start();
  EXPECT_FALSE(bootstrap.is_pending());
  TDS_SuplaDeviceIdentitiesResult result = {SUPLA_SUPLAN_RESULT_OK, 5u};
  bootstrap.on_result(&result);
  EXPECT_FALSE(bootstrap.is_ready());
}

TEST_F(SupLanIdentityBootstrapTest, invalidIdentityCannotStart) {
  EXPECT_CALL(srpc, sd_async_suplan_device_identities(_)).Times(0);
  ids.ChannelCount = SUPLA_CHANNELMAXCOUNT + 1;
  start();
  ids.ChannelCount = -1;
  start();
  ids.ChannelCount = 2;
  ids.ChannelId[1] = 0;
  start();
  ids.ChannelCount = 0;
  ids.DeviceId = 0;
  start();
  EXPECT_FALSE(bootstrap.is_pending());
}

TEST_F(SupLanIdentityBootstrapTest, acceptedEmptyMap) {
  ids.ChannelCount = 0;
  start();
  TDS_SuplaDeviceIdentitiesResult result = {SUPLA_SUPLAN_RESULT_OK, 5u};
  bootstrap.on_result(&result);
  EXPECT_TRUE(bootstrap.is_ready());
}

}  // namespace testing

namespace testing {
TEST_F(SupLanIdentityBootstrapTest, registrationSyncWaitsForDispatchedOk) {
  auto device = std::make_shared<SupLanSyncTestDevice>();
  EXPECT_CALL(*device, send_registration_config()).Times(0);
  device->start_registration_sync(&srpc, &ids);
  EXPECT_EQ(0u, device->get_suplan_root_epoch());
  Mock::VerifyAndClearExpectations(device.get());
  EXPECT_CALL(*device, send_registration_config()).WillOnce([&]() {
    EXPECT_EQ(123u, device->get_suplan_root_epoch());
  });
  TDS_SuplaDeviceIdentitiesResult result = {SUPLA_SUPLAN_RESULT_OK, 123u};
  TsrpcReceivedData rd = {};
  rd.data.ds_suplan_device_identities_result = &result;
  EXPECT_TRUE(device->get_srpc_call_handler_collection()->handle_call(
      device, &srpc, &rd, SUPLA_DS_CALL_SUPLAN_DEVICE_IDENTITIES_RESULT, 29));
  // A duplicate response must never run the registration continuation twice.
  device->on_suplan_device_identities_result(&result);
}

TEST_F(SupLanIdentityBootstrapTest, rejectedBootstrapResumesOrdinarySyncOnly) {
  auto device = std::make_shared<SupLanSyncTestDevice>();
  EXPECT_CALL(*device, send_registration_config()).Times(0);
  device->start_registration_sync(&srpc, &ids);
  Mock::VerifyAndClearExpectations(device.get());
  EXPECT_CALL(*device, send_registration_config()).WillOnce([&]() {
    EXPECT_EQ(0u, device->get_suplan_root_epoch());
  });
  TDS_SuplaDeviceIdentitiesResult result = {
      SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR, 123u};
  device->on_suplan_device_identities_result(&result);
  EXPECT_EQ(0u, device->get_suplan_root_epoch());
}

TEST_F(SupLanIdentityBootstrapTest, malformedBootstrapResumesOrdinarySyncOnly) {
  auto device = std::make_shared<SupLanSyncTestDevice>();
  EXPECT_CALL(*device, send_registration_config()).Times(0);
  device->start_registration_sync(&srpc, &ids);
  Mock::VerifyAndClearExpectations(device.get());
  EXPECT_CALL(*device, send_registration_config()).Times(1);
  device->on_suplan_device_identities_result(nullptr);
  EXPECT_EQ(0u, device->get_suplan_root_epoch());
}

TEST_F(SupLanIdentityBootstrapTest, disconnectCancelsDeferredRegistrationSync) {
  auto device = std::make_shared<SupLanSyncTestDevice>();
  EXPECT_CALL(*device, send_registration_config()).Times(0);
  device->start_registration_sync(&srpc, &ids);
  device->connection_will_close();
  TDS_SuplaDeviceIdentitiesResult result = {SUPLA_SUPLAN_RESULT_OK, 123u};
  device->on_suplan_device_identities_result(&result);
  EXPECT_EQ(0u, device->get_suplan_root_epoch());
}

TEST_F(SupLanIdentityBootstrapTest, repeatedRegistrationCancelsDeferredSync) {
  auto device = std::make_shared<SupLanSyncTestDevice>();
  EXPECT_CALL(*device, send_registration_config()).Times(0);
  device->start_registration_sync(&srpc, &ids);
  supla_register_device registration;
  registration.register_device(device, nullptr, nullptr, &srpc, 0, 0, 20);
  TDS_SuplaDeviceIdentitiesResult result = {SUPLA_SUPLAN_RESULT_OK, 123u};
  device->on_suplan_device_identities_result(&result);
  EXPECT_EQ(0u, device->get_suplan_root_epoch());
}

TEST_F(SupLanIdentityBootstrapTest, ordinaryOrInconsistentSyncsImmediately) {
  for (int flags : {0, SUPLA_DEVICE_FLAG_SYNC_DONE_SUPPORTED,
                   SUPLA_DEVICE_FLAG_SUPLAN_SUPPORTED}) {
    auto device = std::make_shared<SupLanSyncTestDevice>();
    device->registration_flags(flags);
    EXPECT_CALL(srpc, sd_async_suplan_device_identities(_)).Times(0);
    EXPECT_CALL(*device, send_registration_config()).Times(1);
    device->start_registration_sync(&srpc, &ids);
    EXPECT_EQ(0u, device->get_suplan_root_epoch());
    Mock::VerifyAndClearExpectations(&srpc);
  }
}

TEST_F(SupLanIdentityBootstrapTest, failedIdentitySendCannotCompleteSync) {
  auto device = std::make_shared<SupLanSyncTestDevice>();
  EXPECT_CALL(srpc, sd_async_suplan_device_identities(_)).WillOnce(Return(0));
  EXPECT_CALL(srpc, sd_async_device_sync_done()).Times(0);
  EXPECT_CALL(*device, send_registration_config()).Times(0);
  device->start_registration_sync(&srpc, &ids);
  EXPECT_EQ(0u, device->get_suplan_root_epoch());
  TDS_SuplaDeviceIdentitiesResult result = {SUPLA_SUPLAN_RESULT_OK, 123u};
  device->on_suplan_device_identities_result(&result);
  EXPECT_EQ(0u, device->get_suplan_root_epoch());
  device->connection_will_close();
  Mock::VerifyAndClearExpectations(&srpc);
  EXPECT_CALL(srpc, sd_async_suplan_device_identities(_)).WillOnce(Return(1));
  device->start_registration_sync(&srpc, &ids);
  EXPECT_EQ(0u, device->get_suplan_root_epoch());
  Mock::VerifyAndClearExpectations(device.get());
  EXPECT_CALL(*device, send_registration_config()).WillOnce([&]() {
    EXPECT_EQ(123u, device->get_suplan_root_epoch());
  });
  device->on_suplan_device_identities_result(&result);
}

TEST_F(SupLanIdentityBootstrapTest, deviceDispatcherAndCloseUseSameBarrier) {
  auto device = std::make_shared<SupLanSyncTestDevice>();
  device->start_registration_sync(&srpc, &ids);
  EXPECT_EQ(0u, device->get_suplan_root_epoch());
  TDS_SuplaDeviceIdentitiesResult result = {SUPLA_SUPLAN_RESULT_OK, 123u};
  TsrpcReceivedData rd = {};
  rd.data.ds_suplan_device_identities_result = &result;
  EXPECT_TRUE(device->get_srpc_call_handler_collection()->handle_call(
      device, &srpc, &rd, SUPLA_DS_CALL_SUPLAN_DEVICE_IDENTITIES_RESULT, 29));
  EXPECT_EQ(123u, device->get_suplan_root_epoch());
  device->connection_will_close();
  EXPECT_EQ(0u, device->get_suplan_root_epoch());
  device->start_registration_sync(&srpc, &ids);
  device->connection_will_close();
  device->on_suplan_device_identities_result(&result);
  EXPECT_EQ(0u, device->get_suplan_root_epoch());
}

TEST_F(SupLanIdentityBootstrapTest, repeatedRegistrationAbandonsOldExchange) {
  auto device = std::make_shared<SupLanSyncTestDevice>();
  device->start_registration_sync(&srpc, &ids);
  supla_register_device registration;
  registration.register_device(device, nullptr, nullptr, &srpc, 0, 0, 20);
  TDS_SuplaDeviceIdentitiesResult result = {SUPLA_SUPLAN_RESULT_OK, 123u};
  device->on_suplan_device_identities_result(&result);
  EXPECT_EQ(0u, device->get_suplan_root_epoch());
}

TEST_F(SupLanIdentityBootstrapTest, malformedOrOldProtocolResultFailsBarrier) {
  auto device = std::make_shared<SupLanSyncTestDevice>();
  TsrpcReceivedData rd = {};
  device->start_registration_sync(&srpc, &ids);
  device->get_srpc_call_handler_collection()->handle_call(
      device, &srpc, &rd, SUPLA_DS_CALL_SUPLAN_DEVICE_IDENTITIES_RESULT, 29);
  EXPECT_EQ(0u, device->get_suplan_root_epoch());
  device->start_registration_sync(&srpc, &ids);
  TDS_SuplaDeviceIdentitiesResult result = {SUPLA_SUPLAN_RESULT_OK, 123u};
  rd.data.ds_suplan_device_identities_result = &result;
  device->get_srpc_call_handler_collection()->handle_call(
      device, &srpc, &rd, SUPLA_DS_CALL_SUPLAN_DEVICE_IDENTITIES_RESULT, 28);
  EXPECT_EQ(0u, device->get_suplan_root_epoch());
}
}  // namespace testing
