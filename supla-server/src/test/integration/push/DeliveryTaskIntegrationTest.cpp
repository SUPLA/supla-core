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

#include "integration/push/DeliveryTaskIntegrationTest.h"

#include "gmock/gmock.h"
#include "http/asynctask_http_thread_bucket.h"
#include "push/pn_delivery_task.h"
#include "push/push_notification.h"

namespace testing {

using std::shared_ptr;
using std::string;

DeliveryTaskIntegrationTest::DeliveryTaskIntegrationTest(void)
    : IntegrationTest(), AsyncTaskTest() {
  tokenProviderCurlAdapter = nullptr;
  deliveryTaskCurlAdapter = nullptr;
  provider = nullptr;
}

DeliveryTaskIntegrationTest::~DeliveryTaskIntegrationTest(void) {}

void DeliveryTaskIntegrationTest::SetUp(void) {
  AsyncTaskTest::SetUp();

  initTestDatabase();

  tokenProviderCurlAdapter = new CurlAdapterMock();
  provider = new AccessTokenProviderMock(tokenProviderCurlAdapter);

  ON_CALL(*tokenProviderCurlAdapter, perform).WillByDefault(Return(true));
  ON_CALL(*tokenProviderCurlAdapter, get_response_code)
      .WillByDefault(Return(200));

  ON_CALL(*tokenProviderCurlAdapter, set_opt_write_data)
      .WillByDefault([this](string *request_result) {
        *request_result =
            "{\"android\":{\"0\":{\"token\":\"tokenXyz\",\"expires_in\":3600,"
            "\"url\":\"https://"
            "push-fcm.supla.org\"}},\"ios\":{\"0\":{\"token\":\"xcvbn\","
            "\"expires_in\":3600,\"bundle_id\":\"com.supla\",\"url\":\"https://"
            "push-apns.supla.org/{device_token}\",\"development_url\":\"https:/"
            "/devel-push-apns.supla.org/"
            "{device_token}\"},\"200\":{\"token\":\"xcvbn200\",\"expires_in\":"
            "3600,\"bundle_id\":\"com.supla.200\",\"url\":\"https://"
            "push-apns.supla.org/{device_token}\",\"development_url\":\"https:/"
            "/devel-push-apns.supla.org/{device_token}\"}}}";
      });

  provider->refresh();

  deliveryTaskCurlAdapter = new CurlAdapterMock();

  ON_CALL(*deliveryTaskCurlAdapter, perform).WillByDefault(Return(true));
  ON_CALL(*deliveryTaskCurlAdapter, get_response_code)
      .WillByDefault(Return(200));

  ON_CALL(*pool, get_bucket)
      .WillByDefault(Return(
          new supla_asynctask_http_thread_bucket(deliveryTaskCurlAdapter)));
}

void DeliveryTaskIntegrationTest::TearDown(void) {
  delete provider;
  AsyncTaskTest::TearDown();
}

TEST_F(DeliveryTaskIntegrationTest, notificationLoadedFromDatabase) {
  runSqlScript("AddPushNotification.sql");

  EXPECT_CALL(*deliveryTaskCurlAdapter,
              append_header(StrEq("Content-Type: application/json")))
      .Times(1);
  EXPECT_CALL(*deliveryTaskCurlAdapter,
              append_header(StrEq("Authorization: Bearer tokenXyz")))
      .Times(1);

  EXPECT_CALL(*deliveryTaskCurlAdapter,
              set_opt_url(StrEq("https://push-fcm.supla.org")))
      .Times(1);

  EXPECT_CALL(
      *deliveryTaskCurlAdapter,
      set_opt_post_fields(StrEq("{\"message\":{\"token\":\"Token "
                                "1\",\"android\":{\"notification\":{\"title\":"
                                "\"Abcd\",\"body\":\"Efgh\"}}}}")))
      .Times(1);

  EXPECT_CALL(*deliveryTaskCurlAdapter,
              append_header(StrEq("content-type: application/json")))
      .Times(2);

  EXPECT_CALL(*deliveryTaskCurlAdapter,
              append_header(StrEq("authorization: bearer xcvbn")))
      .Times(1);

  EXPECT_CALL(*deliveryTaskCurlAdapter,
              append_header(StrEq("authorization: bearer xcvbn200")))
      .Times(1);

  EXPECT_CALL(*deliveryTaskCurlAdapter,
              append_header(StrEq("apns-topic: com.supla")))
      .Times(1);

  EXPECT_CALL(*deliveryTaskCurlAdapter,
              append_header(StrEq("apns-topic: com.supla.200")))
      .Times(1);

  EXPECT_CALL(*deliveryTaskCurlAdapter,
              append_header(StrEq("apns-push-type: alert")))
      .Times(2);

  EXPECT_CALL(*deliveryTaskCurlAdapter, escape(StrEq("Token 2")))
      .WillOnce(Return("Token%202"));

  EXPECT_CALL(*deliveryTaskCurlAdapter,
              set_opt_url(StrEq("https://push-apns.supla.org/Token%202")))
      .Times(1);

  EXPECT_CALL(*deliveryTaskCurlAdapter, escape(StrEq("Token 3")))
      .WillOnce(Return("Token%203"));

  EXPECT_CALL(*deliveryTaskCurlAdapter,
              set_opt_url(StrEq("https://devel-push-apns.supla.org/Token%203")))
      .Times(1);

  EXPECT_CALL(
      *deliveryTaskCurlAdapter,
      set_opt_post_fields(StrEq(
          "{\"aps\":{\"alert\":{\"title\":\"Abcd\",\"body\":\"Efgh\"}}}")))
      .Times(2);

  shared_ptr<supla_abstract_asynctask> task =
      (new supla_pn_delivery_task(2, queue, pool,
                                  new supla_push_notification(5), provider))
          ->start();

  WaitForState(task, supla_asynctask_state::SUCCESS, 1000000);
}

} /* namespace testing */
