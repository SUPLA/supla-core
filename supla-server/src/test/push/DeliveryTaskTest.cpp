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

#include "DeliveryTaskTest.h"

#include "gmock/gmock.h"
#include "http/asynctask_http_thread_bucket.h"
#include "push/pn_delivery_task.h"
#include "push/push_notification.h"

namespace testing {

using std::shared_ptr;
using std::string;

DeliveryTaskTest::DeliveryTaskTest(void) : AsyncTaskTest() {
  tokenProviderCurlAdapter = nullptr;
  deliveryTaskCurlAdapter = nullptr;
  provider = nullptr;
}

DeliveryTaskTest::~DeliveryTaskTest(void) {}

void DeliveryTaskTest::SetUp(void) {
  AsyncTaskTest::SetUp();

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
            "push-fcm.supla.org\"}},\"ios\":{\"5\":{\"token\":\"xcvbn\","
            "\"expires_in\":3600,\"bundle_id\":\"com.supla\",\"url\":\"https://"
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

  ON_CALL(*deliveryTaskCurlAdapter, escape)
      .WillByDefault([](const std::string &str) { return str; });
}

void DeliveryTaskTest::TearDown(void) {
  delete provider;
  AsyncTaskTest::TearDown();
}

TEST_F(DeliveryTaskTest, recipientsFromAndroidAndiOsPlatforms) {
  supla_push_notification *push = new supla_push_notification();
  push->set_title("TiTle");
  push->set_body("BoDy");

  supla_pn_recipient *r1 = new supla_pn_recipient(
      1, 0, false, "0956469ed2650ed09534e4193ef8028f950");
  supla_pn_recipient *r2 = new supla_pn_recipient(1, 5, false, "2568548549");
  supla_pn_recipient *r3 = new supla_pn_recipient(1, 5, true, "ybuabnuf548549");

  push->get_recipients().add(r1, platform_android);

  push->get_recipients().add(r2, platform_ios);

  push->get_recipients().add(r3, platform_ios);

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
      set_opt_post_fields(StrEq(
          "{\"message\":{\"token\":\"0956469ed2650ed09534e4193ef8028f950\",\"android\":{\"notification\":{\"title\":\"TiTle\",\"body\":\"BoDy\"}}}}")))
      .Times(1);

  EXPECT_CALL(*deliveryTaskCurlAdapter,
              append_header(StrEq("content-type: application/json")))
      .Times(2);

  EXPECT_CALL(*deliveryTaskCurlAdapter,
              append_header(StrEq("authorization: bearer xcvbn")))
      .Times(2);

  EXPECT_CALL(*deliveryTaskCurlAdapter,
              append_header(StrEq("apns-topic: com.supla")))
      .Times(2);

  EXPECT_CALL(*deliveryTaskCurlAdapter,
              append_header(StrEq("apns-push-type: alert")))
      .Times(2);

  EXPECT_CALL(*deliveryTaskCurlAdapter,
              set_opt_url(StrEq("https://push-apns.supla.org/2568548549")))
      .Times(1);

  EXPECT_CALL(
      *deliveryTaskCurlAdapter,
      set_opt_url(StrEq("https://devel-push-apns.supla.org/ybuabnuf548549")))
      .Times(1);

  EXPECT_CALL(
      *deliveryTaskCurlAdapter,
      set_opt_post_fields(StrEq(
          "{\"aps\":{\"alert\":{\"title\":\"TiTle\",\"body\":\"BoDy\"}}}")))
      .Times(2);

  shared_ptr<supla_abstract_asynctask> task =
      (new supla_pn_delivery_task(1, queue, pool, push, provider))->start();

  WaitForState(task, supla_asynctask_state::SUCCESS, 1000000);

  EXPECT_TRUE(r1->is_exists());
  EXPECT_TRUE(r2->is_exists());
  EXPECT_TRUE(r3->is_exists());
}

TEST_F(DeliveryTaskTest, fcmMessageId) {
  supla_push_notification *push = new supla_push_notification();
  push->set_title("T");
  push->set_body("B");

  supla_pn_recipient *r = new supla_pn_recipient(1, 0, false, "0956469");
  push->get_recipients().add(r, platform_android);

  EXPECT_CALL(*deliveryTaskCurlAdapter, set_opt_write_data)
      .WillOnce([this](string *request_result) {
        *request_result = "{\"name\":\"projects/abcdfg/messages/MsgId123\"}";
      });

  shared_ptr<supla_abstract_asynctask> task =
      (new supla_pn_delivery_task(1, queue, pool, push, provider))->start();

  WaitForState(task, supla_asynctask_state::SUCCESS, 1000000);

  EXPECT_EQ(r->get_message_id(), "MsgId123");
}

TEST_F(DeliveryTaskTest, apnsMessageId) {
  supla_push_notification *push = new supla_push_notification();
  push->set_title("T");
  push->set_body("B");

  supla_pn_recipient *r = new supla_pn_recipient(1, 5, false, "0956469");
  push->get_recipients().add(r, platform_ios);

  EXPECT_CALL(*deliveryTaskCurlAdapter, set_opt_header_data)
      .WillOnce([this](std::list<std::string> *data) {
        data->push_back("apns-id: APNS-ID");
      });

  shared_ptr<supla_abstract_asynctask> task =
      (new supla_pn_delivery_task(1, queue, pool, push, provider))->start();

  WaitForState(task, supla_asynctask_state::SUCCESS, 1000000);

  EXPECT_EQ(r->get_message_id(), "APNS-ID");
}

TEST_F(DeliveryTaskTest, fcmRecipientDoesNotExist) {
  supla_push_notification *push = new supla_push_notification();
  push->set_title("T");
  push->set_body("B");

  supla_pn_recipient *r = new supla_pn_recipient(1, 0, false, "0956469");
  push->get_recipients().add(r, platform_android);
  EXPECT_TRUE(r->is_exists());

  EXPECT_CALL(*deliveryTaskCurlAdapter, get_response_code)
      .WillRepeatedly(Return(404));

  shared_ptr<supla_abstract_asynctask> task =
      (new supla_pn_delivery_task(1, queue, pool, push, provider))->start();

  WaitForState(task, supla_asynctask_state::FAILURE, 1000000);

  EXPECT_FALSE(r->is_exists());
}

TEST_F(DeliveryTaskTest, apnsRecipientDoesNotExist) {
  supla_push_notification *push = new supla_push_notification();
  push->set_title("T");
  push->set_body("B");

  supla_pn_recipient *r = new supla_pn_recipient(1, 5, false, "0956469");
  push->get_recipients().add(r, platform_ios);
  EXPECT_TRUE(r->is_exists());

  EXPECT_CALL(*deliveryTaskCurlAdapter, get_response_code)
      .WillRepeatedly(Return(400));

  EXPECT_CALL(*deliveryTaskCurlAdapter, set_opt_write_data)
      .WillOnce([this](string *request_result) {
        *request_result = "{\"reason\":\"BadDeviceToken\"}";
      });

  shared_ptr<supla_abstract_asynctask> task =
      (new supla_pn_delivery_task(1, queue, pool, push, provider))->start();

  WaitForState(task, supla_asynctask_state::FAILURE, 1000000);

  EXPECT_FALSE(r->is_exists());
}

} /* namespace testing */
