// SPDX-FileCopyrightText: AC SOFTWARE SP. Z O.O.
// SPDX-License-Identifier: GPL-2.0-or-later
#include "doubles/SrpcAdapterMock.h"
#include "suplan/identity_bootstrap.h"

namespace testing {
TEST(SupLanIdentityStateTest, LateResultsCannotEraseAcceptedRoot) {
  SrpcAdapterMock srpc;
  ON_CALL(srpc, get_proto_version).WillByDefault(Return(29));
  ON_CALL(srpc, sd_async_suplan_device_identities(_)).WillByDefault(Return(1));
  supla_suplan_identity_bootstrap bootstrap;
  TSD_SuplaDeviceIdentities ids = {};
  ids.DeviceId = 7654;
  auto start = [&] {
    bootstrap.start(&srpc, SUPLA_DEVICE_FLAG_SUPLAN_SUPPORTED |
                               SUPLA_DEVICE_FLAG_SYNC_DONE_SUPPORTED, &ids);
  };
  TDS_SuplaDeviceIdentitiesResult result = {SUPLA_SUPLAN_RESULT_OK, 123};
  bootstrap.on_result(&result);
  EXPECT_FALSE(bootstrap.is_ready());
  start();
  bootstrap.on_result(&result);
  ASSERT_TRUE(bootstrap.is_ready());
  for (unsigned epoch : {123u, 456u, 0u, UINT32_MAX}) {
    result.RootEpoch = epoch;
    bootstrap.on_result(&result);
    EXPECT_EQ(123u, bootstrap.get_root_epoch());
  }
  bootstrap.on_result(nullptr);
  EXPECT_EQ(123u, bootstrap.get_root_epoch());
  bootstrap.reset();
  bootstrap.on_result(&result);
  EXPECT_FALSE(bootstrap.is_ready());
  start();
  result.RootEpoch = 456;
  bootstrap.on_result(&result);
  EXPECT_EQ(456u, bootstrap.get_root_epoch());
}
TEST(SupLanIdentityStateTest, DeadlineExpiresOnceWithoutTrafficOrRetry) {
  NiceMock<SrpcAdapterMock> srpc;
  ON_CALL(srpc, get_proto_version).WillByDefault(Return(29));
  EXPECT_CALL(srpc, sd_async_suplan_device_identities(_))
      .Times(1).WillOnce(Return(1));
  supla_suplan_identity_bootstrap bootstrap;
  TSD_SuplaDeviceIdentities ids = {};
  ids.DeviceId = 7654;
  const auto began = supla_suplan_identity_bootstrap::time_point_t{};
  EXPECT_EQ(0u, bootstrap.time_left_usec(began));
  bootstrap.start(&srpc, SUPLA_DEVICE_FLAG_SUPLAN_SUPPORTED |
                             SUPLA_DEVICE_FLAG_SYNC_DONE_SUPPORTED,
                  &ids, began);
  auto before = began + std::chrono::seconds(15) -
                std::chrono::microseconds(1);
  EXPECT_FALSE(bootstrap.expire(before));
  EXPECT_LE(bootstrap.time_left_usec(before), 2u);
  EXPECT_EQ(1u, bootstrap.time_left_usec(began + std::chrono::seconds(15)));
  EXPECT_TRUE(bootstrap.expire(began + std::chrono::seconds(15)));
  EXPECT_FALSE(bootstrap.is_pending());
  EXPECT_EQ(0u, bootstrap.time_left_usec(began + std::chrono::hours(1)));
  EXPECT_FALSE(bootstrap.expire(began + std::chrono::hours(1)));
  TDS_SuplaDeviceIdentitiesResult late = {SUPLA_SUPLAN_RESULT_OK, 123};
  bootstrap.on_result(&late);
  EXPECT_FALSE(bootstrap.is_ready());
}

TEST(SupLanIdentityStateTest, AcceptedResetAndLegacyHaveNoDeadline) {
  NiceMock<SrpcAdapterMock> srpc;
  ON_CALL(srpc, get_proto_version).WillByDefault(Return(29));
  ON_CALL(srpc, sd_async_suplan_device_identities(_)).WillByDefault(Return(1));
  supla_suplan_identity_bootstrap bootstrap;
  TSD_SuplaDeviceIdentities ids = {};
  ids.DeviceId = 7654;
  auto began = supla_suplan_identity_bootstrap::time_point_t{};
  int flags = SUPLA_DEVICE_FLAG_SUPLAN_SUPPORTED |
              SUPLA_DEVICE_FLAG_SYNC_DONE_SUPPORTED;
  bootstrap.start(&srpc, flags, &ids, began);
  TDS_SuplaDeviceIdentitiesResult ok = {SUPLA_SUPLAN_RESULT_OK, 123};
  bootstrap.on_result(&ok);
  EXPECT_EQ(0u, bootstrap.time_left_usec(began));
  EXPECT_FALSE(bootstrap.expire(began + std::chrono::hours(1)));
  EXPECT_EQ(123u, bootstrap.get_root_epoch());
  bootstrap.start(&srpc, flags, &ids, began);
  bootstrap.reset();
  EXPECT_EQ(0u, bootstrap.time_left_usec(began));
  bootstrap.start(&srpc, 0, &ids, began);
  EXPECT_EQ(0u, bootstrap.time_left_usec(began));
  EXPECT_FALSE(bootstrap.expire(began + std::chrono::hours(1)));
}
}  // namespace testing
