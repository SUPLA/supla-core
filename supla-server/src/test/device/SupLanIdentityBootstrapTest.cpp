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

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <chrono>
#include <cstdio>
#include <functional>
#include <future>
#include <memory>
#include <string>
#include <vector>

#include "conn/connection.h"
#include "device/RegisterDeviceEssentialTest.h"
#include "device/call_handler/register_device.h"
#include "device/device.h"
#include "device/devicechannels.h"
#include "doubles/device/DeviceStub.h"
#include "jsonconfig/channel/weekly_schedule_config.h"
#include "srpc/abstract_srpc_call_hanlder_collection.h"
#include "sthread.h"
#include "supla-socket.h"
#include "suplan/identity_bootstrap.h"

namespace testing {

// M1's transport fixture already supplies Channels through DeviceDaoMock.
// Its authoritative config is that fixture; M3's DB/Grant reconciliation is
// tested independently through the real DAO in SupLanHvacIntegrationTest.
class SupLanReplayHvacChannel : public supla_device_channel {
 protected:
  bool reload_channel_config() override {
    return on_reload ? on_reload() : config_available;
  }

 public:
  bool config_available = true;
  std::function<bool()> on_reload;
  unsigned _supla_int64_t extra_flags = 0;
  bool runtime_config_supported = true;
  unsigned _supla_int64_t get_flags() override {
    auto flags = supla_device_channel::get_flags() | extra_flags;
    return runtime_config_supported
               ? flags
               : flags & ~SUPLA_CHANNEL_FLAG_RUNTIME_CHANNEL_CONFIG_UPDATE;
  }
  using supla_device_channel::supla_device_channel;
};

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

class SupLanReplayDevice : public DeviceStub {
 public:
  explicit SupLanReplayDevice(supla_connection *connection)
      : DeviceStub(connection) {
    set_registered(true);
  }
};

// Use the real registration config sender, ChannelConfig preparation,
// coordinator and SRPC queue. Only the authoritative DB lookup is replaced.
class SupLanRegistrationReplayTest : public Test {
 protected:
  void *listener = nullptr;
  int client = -1;
  supla_connection *connection = nullptr;
  DeviceStub *device = nullptr;
  supla_device_channels *channels = nullptr;
  void *parser = nullptr;
  void *worker = nullptr;
  TSD_SuplaDeviceIdentities identities = {};

  int publication_type = SUPLA_CHANNELTYPE_HVAC;
  int publication_function = SUPLA_CHANNELFNC_HVAC_THERMOSTAT;
  std::string publication_json = "{\"minOnTimeS\":17}";

  void family(int type, int function, const char *json) {
    disconnect_device();
    publication_type = type;
    publication_function = function;
    publication_json = json;
    connect_device();
  }

  void finish_family_sync() {
    start_identity();
    accept_identity();
    auto packets = receive();
    unsigned configs = 0;
    for (const auto &packet : packets) {
      if (packet.call_id != SUPLA_SD_CALL_SET_CHANNEL_CONFIG) {
        continue;
      }
      TSDS_SetChannelConfig config = {};
      memcpy(&config, packet.data, packet.data_size);
      EXPECT_EQ(publication_function, config.Func);
      TSDS_SetChannelConfigResult ack = {};
      ack.ChannelNumber = config.ChannelNumber;
      ack.ConfigType = config.ConfigType;
      ack.Result = SUPLA_CONFIG_RESULT_TRUE;
      channels->on_set_channel_config_result(&ack);
      ++configs;
    }
    EXPECT_GE(configs, 1u);
    expect_sync_done();
  }

  void SetUp() override {
    supla_connection::init();
    listener = ssocket_server_init(nullptr, nullptr, 0, 0);
    ASSERT_NE(nullptr, listener);
    ASSERT_TRUE(ssocket_openlistener(listener));
    identities.DeviceId = 7654;
    identities.ChannelCount = 1;
    identities.ChannelId[0] = 991;
    connect_device();
  }

  void disconnect_device() {
    if (worker) {
      sthread_terminate(worker, false);
      connection->raise_event();
      sthread_wait(worker);
      sthread_free(worker);
      worker = nullptr;
    }
    if (device) {
      device->connection_will_close();
      delete channels;
      channels = nullptr;
      device->set_channels(nullptr);
      delete device;
      device = nullptr;
    }
    delete connection;
    connection = nullptr;
    if (client >= 0) close(client);
    client = -1;
    if (parser) sproto_free(parser);
    parser = nullptr;
  }

  void TearDown() override {
    disconnect_device();
    if (listener) ssocket_free(listener);
    supla_connection::cleanup();
  }

  void connect_device() {
    sockaddr_in address = {};
    socklen_t size = sizeof(address);
    ASSERT_EQ(0, getsockname(ssocket_get_fd(listener),
                            reinterpret_cast<sockaddr *>(&address), &size));
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    client = socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_GE(client, 0);
    ASSERT_EQ(0, connect(client, reinterpret_cast<sockaddr *>(&address), size));
    void *accepted = nullptr;
    unsigned int ip = 0;
    ASSERT_TRUE(ssocket_accept(listener, &ip, &accepted));
    connection = new supla_connection(listener, accepted, ip);
    int no_delay = 1;
    ASSERT_EQ(0, setsockopt(connection->get_client_sd(), IPPROTO_TCP,
                            TCP_NODELAY, &no_delay, sizeof(no_delay)));
    connection->get_srpc_adapter()->set_proto_version(29);
    device = new SupLanReplayDevice(connection);
    device->set_id(7654);
    device->set_flags(SUPLA_DEVICE_FLAG_SUPLAN_SUPPORTED |
                      SUPLA_DEVICE_FLAG_SYNC_DONE_SUPPORTED);
    DeviceDaoMock dao;
    EXPECT_CALL(dao, get_channels(device)).WillOnce([&](supla_device *) {
      char value[SUPLA_CHANNELVALUE_SIZE] = {};
      return std::vector<supla_device_channel *>{new SupLanReplayHvacChannel(
          device, 991, 9, publication_type, publication_function, 0, 0, 0, 0,
          nullptr, nullptr, nullptr, false,
          SUPLA_CHANNEL_FLAG_RUNTIME_CHANNEL_CONFIG_UPDATE, value, 0, nullptr,
          publication_json.c_str(), "{}", nullptr)};
    });
    channels = new supla_device_channels(&dao, device, nullptr, nullptr, 0);
    device->set_channels(channels);
    parser = sproto_init();
    ASSERT_NE(nullptr, parser);
    sthread_simple_run(
        [](void *conn, void *thread) {
          static_cast<supla_connection *>(conn)->execute(thread);
        },
        connection, false, &worker);
    ASSERT_NE(nullptr, worker);
    // Receiving a worker-flushed packet proves execute() initialized its
    // thread context, including the production terminate() path.
    ASSERT_GT(connection->get_srpc_adapter()->sdc_async_ping_server_result(),
              0);
    pollfd fd = {client, POLLIN, 0};
    ASSERT_GT(poll(&fd, 1, 1000), 0);
    char bytes[256];
    int received = recv(client, bytes, sizeof(bytes), 0);
    ASSERT_GT(received, 0);
    ASSERT_EQ(SUPLA_RESULT_TRUE,
              sproto_in_buffer_append(parser, bytes, received));
    TSuplaDataPacket packet = {};
    ASSERT_EQ(SUPLA_RESULT_TRUE, sproto_pop_in_sdp(parser, &packet));
    ASSERT_EQ(SUPLA_SDC_CALL_PING_SERVER_RESULT, packet.call_id);
  }

  std::vector<TSuplaDataPacket> receive() {
    auto adapter = connection->get_srpc_adapter();
    auto srpc = adapter->get_srpc();
    // Flush the actual bounded queue onto TCP before inspecting the packets.
    adapter->lock();
    for (int i = 0; i < 32; ++i) {
      srpc_iterate(srpc);
      if (!srpc_out_queue_item_count(srpc)) break;
    }
    srpc_iterate(srpc);
    adapter->unlock();
    std::vector<TSuplaDataPacket> packets;
    pollfd fd = {client, POLLIN, 0};
    while (poll(&fd, 1, 20) > 0) {
      char bytes[4096];
      int size = recv(client, bytes, sizeof(bytes), MSG_DONTWAIT);
      if (size <= 0) break;
      EXPECT_EQ(SUPLA_RESULT_TRUE,
                sproto_in_buffer_append(parser, bytes, size));
      TSuplaDataPacket packet = {};
      while (sproto_pop_in_sdp(parser, &packet) == SUPLA_RESULT_TRUE) {
        packets.push_back(packet);
      }
    }
    return packets;
  }

  void start_identity() {
    device->start_registration_sync(connection->get_srpc_adapter(),
                                    &identities);
    auto packets = receive();
    ASSERT_EQ(1U, packets.size());
    EXPECT_EQ(SUPLA_SD_CALL_SUPLAN_DEVICE_IDENTITIES, packets[0].call_id);
    EXPECT_EQ(0U, device->get_suplan_root_epoch());
  }

  void accept_identity() {
    TDS_SuplaDeviceIdentitiesResult result = {SUPLA_SUPLAN_RESULT_OK, 123u};
    device->on_suplan_device_identities_result(&result);
    EXPECT_EQ(123U, device->get_suplan_root_epoch());
  }

  TSDS_SetChannelConfig expect_config() {
    auto packets = receive();
    EXPECT_EQ(2U, packets.size());
    TSDS_SetChannelConfig config = {};
    if (packets.size() != 2) return config;
    EXPECT_EQ(SUPLA_SD_CALL_SET_CHANNEL_CONFIG, packets[0].call_id);
    EXPECT_EQ(SUPLA_SD_CALL_CHANNEL_CONFIG_FINISHED, packets[1].call_id);
    memcpy(&config, packets[0].data, packets[0].data_size);
    EXPECT_EQ(9, config.ChannelNumber);
    EXPECT_EQ(SUPLA_CONFIG_TYPE_DEFAULT, config.ConfigType);
    EXPECT_EQ(sizeof(TChannelConfig_HVAC), config.ConfigSize);
    TChannelConfig_HVAC hvac = {};
    memcpy(&hvac, config.Config, sizeof(hvac));
    EXPECT_EQ(17, hvac.MinOnTimeS);
    return config;
  }

  void config_result() {
    TSDS_SetChannelConfigResult result = {};
    result.ChannelNumber = 9;
    result.ConfigType = SUPLA_CONFIG_TYPE_DEFAULT;
    result.Result = SUPLA_CONFIG_RESULT_TRUE;
    channels->on_set_channel_config_result(&result);
  }

  void expect_sync_done() {
    auto packets = receive();
    ASSERT_EQ(1U, packets.size());
    EXPECT_EQ(SUPLA_SD_CALL_DEVICE_SYNC_DONE, packets[0].call_id);
  }
};

TEST_F(SupLanRegistrationReplayTest, identityOkThenConfigThenSyncDone) {
  start_identity();
  accept_identity();
  expect_config();
  EXPECT_TRUE(receive().empty());
  config_result();
  expect_sync_done();
}

TEST_F(SupLanRegistrationReplayTest, disconnectBeforeConfigDeliveryReplays) {
  start_identity();
  accept_identity();
  // Drop the connection before the queued configuration reaches the Device.
  disconnect_device();
  connect_device();
  start_identity();
  accept_identity();
  expect_config();
  config_result();
  expect_sync_done();
}

TEST_F(SupLanRegistrationReplayTest,
       interruptedAndCompletedSyncReplayUnchanged) {
  start_identity();
  accept_identity();
  auto original = expect_config();
  // Crash after identity OK but before ChannelConfig ACK / SYNC_DONE.
  disconnect_device();
  for (int i = 0; i < 2; ++i) {
    connect_device();
    start_identity();
    accept_identity();
    auto replay = expect_config();
    EXPECT_EQ(0, memcmp(&original, &replay, sizeof(original)));
    config_result();
    expect_sync_done();
    // Even a completed sync must not suppress replay at next registration.
    disconnect_device();
  }
}

TEST_F(SupLanRegistrationReplayTest,
       failedConfigPreparationCannotCompleteSync) {
  channels->access_channel(991, [](supla_device_channel *channel) {
    static_cast<SupLanReplayHvacChannel *>(channel)->config_available = false;
  });
  start_identity();
  TDS_SuplaDeviceIdentitiesResult result = {SUPLA_SUPLAN_RESULT_OK, 123u};
  device->on_suplan_device_identities_result(&result);
  EXPECT_EQ(0U, device->get_suplan_root_epoch());
  config_result();
  channels->iterate();
  for (auto &packet : receive()) {
    EXPECT_NE(SUPLA_SD_CALL_DEVICE_SYNC_DONE, packet.call_id);
    EXPECT_NE(SUPLA_SD_CALL_SET_CHANNEL_CONFIG, packet.call_id);
    EXPECT_NE(SUPLA_SD_CALL_CHANNEL_CONFIG_FINISHED, packet.call_id);
  }
  EXPECT_TRUE(sthread_isterminated(worker));
  disconnect_device();
  connect_device();
  start_identity();
  accept_identity();
  expect_config();
  config_result();
  expect_sync_done();
}

TEST_F(SupLanRegistrationReplayTest, failedConfigEnqueueCannotCompleteSync) {
  start_identity();
  // Fill the real SRPC queue to force configuration enqueue failure.
  auto adapter = connection->get_srpc_adapter();
  adapter->lock();
  for (int i = 0; i < 10; ++i) {
    EXPECT_GT(adapter->sdc_async_ping_server_result(), 0);
  }
  TDS_SuplaDeviceIdentitiesResult result = {SUPLA_SUPLAN_RESULT_OK, 123u};
  device->on_suplan_device_identities_result(&result);
  EXPECT_EQ(0U, device->get_suplan_root_epoch());
  config_result();
  channels->iterate();
  for (auto &packet : receive()) {
    EXPECT_NE(SUPLA_SD_CALL_DEVICE_SYNC_DONE, packet.call_id);
    EXPECT_NE(SUPLA_SD_CALL_SET_CHANNEL_CONFIG, packet.call_id);
  }
  EXPECT_TRUE(sthread_isterminated(worker));
  adapter->unlock();
  disconnect_device();
  connect_device();
  start_identity();
  accept_identity();
  expect_config();
  config_result();
  expect_sync_done();
}

TEST_F(SupLanRegistrationReplayTest, failedConfigFinishedCannotCompleteSync) {
  start_identity();
  auto adapter = connection->get_srpc_adapter();
  adapter->lock();
  // Leave room for the config, but not CHANNEL_CONFIG_FINISHED.
  for (int i = 0; i < 9; ++i) {
    EXPECT_GT(adapter->sdc_async_ping_server_result(), 0);
  }
  TDS_SuplaDeviceIdentitiesResult result = {SUPLA_SUPLAN_RESULT_OK, 123u};
  device->on_suplan_device_identities_result(&result);
  EXPECT_EQ(0U, device->get_suplan_root_epoch());
  auto packets = receive();
  EXPECT_EQ(10U, packets.size());
  if (!packets.empty()) {
    EXPECT_EQ(SUPLA_SD_CALL_SET_CHANNEL_CONFIG, packets.back().call_id);
  }
  config_result();
  channels->iterate();
  EXPECT_TRUE(receive().empty());
  EXPECT_TRUE(sthread_isterminated(worker));
  adapter->unlock();
}

TEST_F(SupLanRegistrationReplayTest, missingAuthoritativeChannelFailsClosed) {
  delete channels;
  DeviceDaoMock dao;
  EXPECT_CALL(dao, get_channels(device))
      .WillOnce(Return(std::vector<supla_device_channel *>{}));
  channels = new supla_device_channels(&dao, device, nullptr, nullptr, 0);
  device->set_channels(channels);
  auto adapter = connection->get_srpc_adapter();
  adapter->lock();
  device->start_registration_sync(adapter, &identities);
  EXPECT_EQ(0U, device->get_suplan_root_epoch());
  EXPECT_TRUE(sthread_isterminated(worker));
  TDS_SuplaDeviceIdentitiesResult result = {SUPLA_SUPLAN_RESULT_OK, 123u};
  device->on_suplan_device_identities_result(&result);
  EXPECT_EQ(0U, device->get_suplan_root_epoch());
  for (auto &packet : receive()) {
    EXPECT_NE(SUPLA_SD_CALL_DEVICE_SYNC_DONE, packet.call_id);
  }
  adapter->unlock();
  disconnect_device();
  connect_device();
  start_identity();
  accept_identity();
  expect_config();
  config_result();
  expect_sync_done();
}

TEST_F(SupLanRegistrationReplayTest,
       rejectedIdentityStillAllowsOrdinaryConfig) {
  start_identity();
  TDS_SuplaDeviceIdentitiesResult result = {
      SUPLA_SUPLAN_RESULT_PERSISTENCE_ERROR, 0};
  device->on_suplan_device_identities_result(&result);
  EXPECT_EQ(0U, device->get_suplan_root_epoch());
  expect_config();
  config_result();
  expect_sync_done();
}
// The fixture's socket worker only flushes SRPC; this test thread represents
// the Device connection owner. Producers never load config or touch its cache.
TEST_F(SupLanRegistrationReplayTest,
       pendingPushWaitsForReplayAndCoalesces) {
  start_identity();
  channels->access_channel(991, [](supla_device_channel *channel) {
    for (int i = 0; i < 100; ++i)
      EXPECT_TRUE(
          channel->request_config_publication(SUPLA_CONFIG_TYPE_DEFAULT));
    EXPECT_FALSE(channel->request_config_publication(255));
  });
  channels->iterate();
  EXPECT_TRUE(receive().empty());
  accept_identity();
  expect_config();
  channels->iterate();
  EXPECT_TRUE(receive().empty());
  config_result();
  expect_sync_done();
  channels->iterate();
  auto packets = receive();
  ASSERT_EQ(1U, packets.size());
  EXPECT_EQ(SUPLA_SD_CALL_SET_CHANNEL_CONFIG, packets[0].call_id);
  channels->iterate();
  EXPECT_TRUE(receive().empty());
}

TEST_F(SupLanRegistrationReplayTest,
       stalledReadCannotBlockProducersOrReplayOld) {
  start_identity();
  accept_identity();
  expect_config();
  config_result();
  expect_sync_done();
  SupLanReplayHvacChannel *channel = nullptr;
  channels->access_channel(991, [&](supla_device_channel *c) {
    channel = static_cast<SupLanReplayHvacChannel *>(c);
  });
  ASSERT_NE(nullptr, channel);
  std::promise<void> reading, resume;
  auto resumed = resume.get_future().share();
  int loads = 0;
  channel->on_reload = [&]() {
    if (++loads == 1) {
      reading.set_value();
      resumed.wait();
      // The paused publication completes its older authoritative read first.
      auto config = new supla_json_config();
      config->set_user_config("{\"minOnTimeS\":17}");
      channel->set_json_config(config);
    } else {
      auto config = new supla_json_config();
      config->set_user_config("{\"minOnTimeS\":23}");
      channel->set_json_config(config);
    }
    return true;
  };
  auto owner = std::async(std::launch::async, [&]() {
    TDS_GetChannelConfigRequest request = {};
    request.ChannelNumber = 9;
    request.ConfigType = SUPLA_CONFIG_TYPE_DEFAULT;
    TsrpcReceivedData rd = {};
    rd.data.ds_get_channel_config_request = &request;
    auto borrowed =
        std::shared_ptr<supla_device>(device, [](supla_device *) {});
    return device->get_srpc_call_handler_collection()->handle_call(
        borrowed, connection->get_srpc_adapter(), &rd,
        SUPLA_DS_CALL_GET_CHANNEL_CONFIG, 29);
  });
  auto started = reading.get_future().wait_for(std::chrono::seconds(2));
  auto producer = std::async(std::launch::async, [&]() {
    bool ok = true;
    for (int i = 0; i < 100; ++i)
      ok &= channel->request_config_publication(SUPLA_CONFIG_TYPE_DEFAULT);
    return ok;
  });
  auto submitted = producer.wait_for(std::chrono::seconds(2));
  // Always release the owner, even on a failed readiness assertion.
  resume.set_value();
  EXPECT_EQ(std::future_status::ready, started);
  EXPECT_EQ(std::future_status::ready, submitted);
  EXPECT_TRUE(producer.get());
  EXPECT_TRUE(owner.get());
  channels->iterate();
  auto packets = receive();
  ASSERT_EQ(2U, packets.size());
  for (size_t i = 0; i < packets.size(); ++i) {
    EXPECT_EQ(i ? SUPLA_SD_CALL_SET_CHANNEL_CONFIG
                : SUPLA_SD_CALL_GET_CHANNEL_CONFIG_RESULT,
              packets[i].call_id);
    TSDS_SetChannelConfig config = {};
    memcpy(&config, packets[i].data, packets[i].data_size);
    TChannelConfig_HVAC hvac = {};
    memcpy(&hvac, config.Config, sizeof(hvac));
    EXPECT_EQ(i ? 23 : 17, hvac.MinOnTimeS);
  }
  EXPECT_EQ(2, loads);
  channels->iterate();
  EXPECT_TRUE(receive().empty());
}

TEST_F(SupLanRegistrationReplayTest,
       pendingPushReadFailureDisconnectsAndReplays) {
  start_identity();
  accept_identity();
  expect_config();
  config_result();
  expect_sync_done();
  channels->access_channel(991, [](supla_device_channel *channel) {
    static_cast<SupLanReplayHvacChannel *>(channel)->config_available = false;
    EXPECT_TRUE(channel->request_config_publication(SUPLA_CONFIG_TYPE_DEFAULT));
  });
  channels->iterate();
  EXPECT_EQ(0U, device->get_suplan_root_epoch());
  EXPECT_TRUE(sthread_isterminated(worker));
  EXPECT_TRUE(receive().empty());
  disconnect_device();
  connect_device();
  start_identity();
  accept_identity();
  expect_config();
  config_result();
  expect_sync_done();
}

TEST_F(SupLanRegistrationReplayTest,
       pendingPushFullQueueDisconnectsAndReplays) {
  start_identity();
  accept_identity();
  expect_config();
  config_result();
  expect_sync_done();
  auto adapter = connection->get_srpc_adapter();
  adapter->lock();
  for (int i = 0; i < 10; ++i)
    EXPECT_GT(adapter->sdc_async_ping_server_result(), 0);
  channels->access_channel(991, [](supla_device_channel *channel) {
    EXPECT_TRUE(channel->request_config_publication(SUPLA_CONFIG_TYPE_DEFAULT));
  });
  channels->iterate();
  EXPECT_EQ(0U, device->get_suplan_root_epoch());
  EXPECT_TRUE(sthread_isterminated(worker));
  for (auto &packet : receive())
    EXPECT_NE(SUPLA_SD_CALL_SET_CHANNEL_CONFIG, packet.call_id);
  adapter->unlock();
  disconnect_device();
  connect_device();
  start_identity();
  accept_identity();
  expect_config();
  config_result();
  expect_sync_done();
}
TEST_F(SupLanRegistrationReplayTest, pendingScheduleLoadsAuthoritativeRoot) {
  start_identity();
  accept_identity();
  expect_config();
  config_result();
  expect_sync_done();
  int loads = 0;
  channels->access_channel(991, [&](supla_device_channel *c) {
    auto channel = static_cast<SupLanReplayHvacChannel *>(c);
    channel->extra_flags = SUPLA_CHANNEL_FLAG_WEEKLY_SCHEDULE;
    channel->on_reload = [&, channel]() {
      ++loads;
      auto config = new weekly_schedule_config();
      TChannelConfig_WeeklySchedule weekly = {};
      weekly.Program[0].Mode = SUPLA_HVAC_MODE_HEAT;
      weekly.Program[0].SetpointTemperatureHeat = 2300;
      config->set_config(&weekly, SUPLA_CHANNELFNC_HVAC_THERMOSTAT);
      channel->set_json_config(config);
      return true;
    };
    EXPECT_TRUE(channel->request_config_publication(SUPLA_CONFIG_TYPE_DEFAULT));
    EXPECT_TRUE(
        channel->request_config_publication(SUPLA_CONFIG_TYPE_WEEKLY_SCHEDULE));
    EXPECT_TRUE(
        channel->request_config_publication(SUPLA_CONFIG_TYPE_WEEKLY_SCHEDULE));
  });
  channels->iterate();
  auto first = receive();
  ASSERT_EQ(1U, first.size());
  TSDS_SetChannelConfig config = {};
  memcpy(&config, first[0].data, first[0].data_size);
  EXPECT_EQ(SUPLA_CONFIG_TYPE_DEFAULT, config.ConfigType);
  // Even another default intent cannot starve the pending schedule.
  channels->access_channel(991, [](supla_device_channel *channel) {
    EXPECT_TRUE(channel->request_config_publication(SUPLA_CONFIG_TYPE_DEFAULT));
  });
  channels->iterate();
  auto second = receive();
  ASSERT_EQ(1U, second.size());
  EXPECT_EQ(SUPLA_SD_CALL_SET_CHANNEL_CONFIG, second[0].call_id);
  memcpy(&config, second[0].data, second[0].data_size);
  EXPECT_EQ(SUPLA_CONFIG_TYPE_WEEKLY_SCHEDULE, config.ConfigType);
  ASSERT_EQ(sizeof(TChannelConfig_WeeklySchedule), config.ConfigSize);
  TChannelConfig_WeeklySchedule weekly = {};
  memcpy(&weekly, config.Config, sizeof(weekly));
  EXPECT_EQ(SUPLA_HVAC_MODE_HEAT, weekly.Program[0].Mode);
  EXPECT_EQ(2300, weekly.Program[0].SetpointTemperatureHeat);
  EXPECT_EQ(2, loads);
  channels->iterate();
  auto last = receive();
  ASSERT_EQ(1U, last.size());
  memcpy(&config, last[0].data, last[0].data_size);
  EXPECT_EQ(SUPLA_CONFIG_TYPE_DEFAULT, config.ConfigType);
  EXPECT_EQ(3, loads);
  channels->iterate();
  EXPECT_TRUE(receive().empty());
}

TEST_F(SupLanRegistrationReplayTest, unsupportedPushDoesNotDisconnect) {
  start_identity();
  accept_identity();
  expect_config();
  config_result();
  expect_sync_done();
  channels->access_channel(991, [](supla_device_channel *channel) {
    static_cast<SupLanReplayHvacChannel *>(channel)->runtime_config_supported =
        false;
    EXPECT_TRUE(channel->request_config_publication(SUPLA_CONFIG_TYPE_DEFAULT));
  });
  channels->iterate();
  EXPECT_EQ(123U, device->get_suplan_root_epoch());
  EXPECT_FALSE(sthread_isterminated(worker));
  EXPECT_TRUE(receive().empty());
}
TEST_F(SupLanRegistrationReplayTest, absentPendingSchedulesDoNotDisconnect) {
  start_identity();
  accept_identity();
  expect_config();
  config_result();
  expect_sync_done();
  channels->access_channel(991, [](supla_device_channel *c) {
    auto channel = static_cast<SupLanReplayHvacChannel *>(c);
    channel->extra_flags = SUPLA_CHANNEL_FLAG_WEEKLY_SCHEDULE;
    EXPECT_TRUE(
        channel->request_config_publication(SUPLA_CONFIG_TYPE_WEEKLY_SCHEDULE));
    EXPECT_TRUE(channel->request_config_publication(
        SUPLA_CONFIG_TYPE_ALT_WEEKLY_SCHEDULE));
  });
  channels->iterate();
  channels->iterate();
  EXPECT_TRUE(receive().empty());
  EXPECT_EQ(123U, device->get_suplan_root_epoch());
  EXPECT_FALSE(sthread_isterminated(worker));
  // The missing optional configs do not prevent the next ordinary update.
  channels->access_channel(991, [](supla_device_channel *channel) {
    EXPECT_TRUE(channel->request_config_publication(SUPLA_CONFIG_TYPE_DEFAULT));
  });
  channels->iterate();
  auto packets = receive();
  ASSERT_EQ(1U, packets.size());
  EXPECT_EQ(SUPLA_SD_CALL_SET_CHANNEL_CONFIG, packets[0].call_id);
}

TEST_F(SupLanRegistrationReplayTest, pendingScheduleReadFailureDisconnects) {
  start_identity();
  accept_identity();
  expect_config();
  config_result();
  expect_sync_done();
  channels->access_channel(991, [](supla_device_channel *c) {
    auto channel = static_cast<SupLanReplayHvacChannel *>(c);
    channel->extra_flags = SUPLA_CHANNEL_FLAG_WEEKLY_SCHEDULE;
    channel->config_available = false;
    EXPECT_TRUE(
        channel->request_config_publication(SUPLA_CONFIG_TYPE_WEEKLY_SCHEDULE));
  });
  channels->iterate();
  EXPECT_TRUE(receive().empty());
  EXPECT_EQ(0U, device->get_suplan_root_epoch());
  EXPECT_TRUE(sthread_isterminated(worker));
}

TEST_F(SupLanRegistrationReplayTest,
       optionalReadFailureAbortsEntireReplayBatch) {
  channels->access_channel(991, [](supla_device_channel *c) {
    auto channel = static_cast<SupLanReplayHvacChannel *>(c);
    channel->extra_flags = SUPLA_CHANNEL_FLAG_WEEKLY_SCHEDULE;
    channel->on_reload = [count = 0]() mutable { return ++count == 1; };
  });
  start_identity();
  TDS_SuplaDeviceIdentitiesResult result = {SUPLA_SUPLAN_RESULT_OK, 123u};
  device->on_suplan_device_identities_result(&result);
  // Default preparation succeeded, but the optional config reload failed.
  EXPECT_TRUE(receive().empty());
  EXPECT_EQ(0U, device->get_suplan_root_epoch());
  EXPECT_TRUE(sthread_isterminated(worker));
  config_result();
  channels->iterate();
  EXPECT_TRUE(receive().empty());
}

TEST_F(SupLanRegistrationReplayTest,
       M4ContainerValveFaultRecoveryAndReplayOverlap) {
  for (bool container : {true, false}) {
    SCOPED_TRACE(container ? "Container" : "Valve");
    family(container ? SUPLA_CHANNELTYPE_CONTAINER
                     : SUPLA_CHANNELTYPE_VALVE_OPENCLOSE,
           container ? SUPLA_CHANNELFNC_CONTAINER
                     : SUPLA_CHANNELFNC_VALVE_OPENCLOSE,
           container ? "{\"warningAboveLevel\":16}"
                     : "{\"floodSensorChannelIds\":[]}");
    start_identity();
    channels->access_channel(991, [](supla_device_channel *c) {
      for (int n = 0; n < 100; ++n) {
        ASSERT_TRUE(c->request_config_publication(SUPLA_CONFIG_TYPE_DEFAULT));
      }
    });
    channels->iterate();
    EXPECT_TRUE(receive().empty());
    accept_identity();
    auto replay = receive();
    ASSERT_EQ(2u, replay.size());
    channels->iterate();
    EXPECT_TRUE(receive().empty());  // pending update cannot consume replay ACK
    config_result();
    expect_sync_done();
    channels->iterate();
    auto update = receive();
    ASSERT_EQ(1u, update.size());
    EXPECT_EQ(SUPLA_SD_CALL_SET_CHANNEL_CONFIG, update[0].call_id);
    channels->iterate();
    EXPECT_TRUE(receive().empty());
    // Optional absent/unsupported schedule leaves accepted identity intact.
    channels->access_channel(991, [](supla_device_channel *c) {
      c->request_config_publication(SUPLA_CONFIG_TYPE_WEEKLY_SCHEDULE);
      c->request_config_publication(SUPLA_CONFIG_TYPE_ALT_WEEKLY_SCHEDULE);
    });
    channels->iterate();
    channels->iterate();
    EXPECT_TRUE(receive().empty());
    EXPECT_EQ(123u, device->get_suplan_root_epoch());
    channels->access_channel(991, [](supla_device_channel *c) {
      static_cast<SupLanReplayHvacChannel *>(c)->config_available = false;
      c->request_config_publication(SUPLA_CONFIG_TYPE_DEFAULT);
    });
    channels->iterate();
    EXPECT_EQ(0u, device->get_suplan_root_epoch());
    EXPECT_TRUE(sthread_isterminated(worker));
    EXPECT_TRUE(receive().empty());
    disconnect_device();
    connect_device();
    finish_family_sync();
    // Actual bounded SRPC queue overflow uses the same reconnect recovery.
    auto adapter = connection->get_srpc_adapter();
    adapter->lock();
    for (int n = 0; n < 10; ++n) {
      srpc_sdc_async_ping_server_result(adapter->get_srpc());
    }
    channels->access_channel(991, [](supla_device_channel *c) {
      c->request_config_publication(SUPLA_CONFIG_TYPE_DEFAULT);
    });
    channels->iterate();
    adapter->unlock();
    EXPECT_EQ(0u, device->get_suplan_root_epoch());
    EXPECT_TRUE(sthread_isterminated(worker));
    for (const auto &packet : receive()) {
      EXPECT_NE(SUPLA_SD_CALL_SET_CHANNEL_CONFIG, packet.call_id);
    }
    disconnect_device();
    connect_device();
    finish_family_sync();
  }
}

TEST_F(SupLanRegistrationReplayTest, M4OcrExtendedPublicationAndFairSelection) {
  for (bool ocr : {true, false}) {
    SCOPED_TRACE(ocr ? "OCR" : "EXTENDED");
    family(ocr ? SUPLA_CHANNELTYPE_IMPULSE_COUNTER : SUPLA_CHANNELTYPE_RELAY,
           ocr ? SUPLA_CHANNELFNC_IC_ELECTRICITY_METER
               : SUPLA_CHANNELFNC_STAIRCASETIMER,
           ocr ? "{\"ocr\":{\"photoIntervalSec\":60}}"
               : "{\"relayTimeMs\":5000,\"overcurrentThreshold\":10}");
    finish_family_sync();
    auto wanted = ocr ? SUPLA_CONFIG_TYPE_OCR : SUPLA_CONFIG_TYPE_EXTENDED;
    channels->access_channel(991, [&](supla_device_channel *c) {
      EXPECT_TRUE(c->request_config_publication(wanted));
      c->request_config_publication(SUPLA_CONFIG_TYPE_DEFAULT);
    });
    bool seen = false;
    for (int n = 0; n < 6; ++n) {
      channels->access_channel(991, [](supla_device_channel *c) {
        c->request_config_publication(SUPLA_CONFIG_TYPE_DEFAULT);
      });
      channels->iterate();
      for (const auto &packet : receive()) {
        ASSERT_EQ(SUPLA_SD_CALL_SET_CHANNEL_CONFIG, packet.call_id);
        TSDS_SetChannelConfig config = {};
        memcpy(&config, packet.data, packet.data_size);
        if (config.ConfigType == wanted) {
          seen = true;
          EXPECT_GT(config.ConfigSize, 0);
          EXPECT_EQ(ocr ? sizeof(TChannelConfig_OCR)
                        : sizeof(TChannelConfig_PowerSwitch),
                    config.ConfigSize);
        }
      }
    }
    EXPECT_TRUE(seen);  // recurring DEFAULT cannot starve higher config types
    EXPECT_EQ(123u, device->get_suplan_root_epoch());
  }
}

}  // namespace testing
