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

#include <list>
#include <memory>
#include <string>
#include <vector>

#include "device/value/channel_binary_sensor_value.h"
#include "doubles/device/ChannelPropertyGetterMock.h"
#include "gmock/gmock.h"
#include "http/asynctask_http_thread_bucket.h"
#include "push/pn_delivery_task.h"
#include "push/push_notification.h"

namespace testing {

using std::shared_ptr;
using std::string;
using std::vector;

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
      .WillByDefault([this](int instance_id, string *request_result) {
        *request_result =
            "{\"push_android\":{\"0\":{\"token\":\"tokenXyz\",\"expires_in\":"
            "3600,"
            "\"url\":\"https://"
            "push-fcm.supla.org\"}},\"push_ios\":{\"5\":{\"token\":\"xcvbn\","
            "\"expires_in\":3600,\"bundle_id\":\"com.supla\",\"url\":\"https://"
            "push-apns.supla.org/{device_token}\",\"development_url\":\"https:/"
            "/devel-push-apns.supla.org/{device_token}\"}, "
            "\"6\":{\"token\":\"xcooo\",\"expires_in\":3600,\"bundle_id\":"
            "\"com.supla\",\"url\":\"https://push-apns.supla.org/"
            "{device_token}\",\"development_url\":\"https://"
            "devel-push-apns.supla.org/{device_token}\"}}}";
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
      .WillByDefault(
          [](int instance_id, const std::string &str) { return str; });

  ON_CALL(throttling, is_delivery_possible).WillByDefault(Return(true));
  EXPECT_CALL(throttling, on_message_not_sent).Times(0);
}

void DeliveryTaskTest::TearDown(void) {
  delete provider;
  AsyncTaskTest::TearDown();
}

TEST_F(DeliveryTaskTest, recipientsFromAndroidAndiOsPlatforms) {
  supla_push_notification *push = new supla_push_notification();
  push->set_title("TiTle");
  push->set_body("BoDy");
  push->set_localized_title("Localized Title");
  push->set_localized_body("Localized Body");

  vector<string> targs{"t1", "t2"};
  vector<string> bargs{"b1", "b2"};

  push->set_localized_title_args(targs);
  push->set_localized_body_args(bargs);

  supla_pn_recipient *r1 = new supla_pn_recipient(
      1, 0, false, "0956469ed2650ed09534e4193ef8028f950", "My Profile 123", 0);
  supla_pn_recipient *r2 =
      new supla_pn_recipient(1, 5, false, "2568548549", "ABCD", 0);
  supla_pn_recipient *r3 =
      new supla_pn_recipient(1, 6, true, "ybuabnuf548549", "ABCD", 0);

  push->get_recipients().add(r1, platform_push_android);

  push->get_recipients().add(r2, platform_push_ios);

  push->get_recipients().add(r3, platform_push_ios);

  EXPECT_CALL(*deliveryTaskCurlAdapter,
              append_header(Eq(0), StrEq("Content-Type: application/json")))
      .Times(1);
  EXPECT_CALL(*deliveryTaskCurlAdapter,
              append_header(Eq(0), StrEq("Authorization: Bearer tokenXyz")))
      .Times(1);

  EXPECT_CALL(*deliveryTaskCurlAdapter,
              set_opt_url(Eq(0), StrEq("https://push-fcm.supla.org")))
      .Times(1);

  EXPECT_CALL(
      *deliveryTaskCurlAdapter,
      set_opt_post_fields(
          Eq(0),
          StrEq(
              "{\"message\":{\"token\":\"0956469ed2650ed09534e4193ef8028f950\","
              "\"android\":{\"priority\":\"high\",\"notification\":{\"title\":"
              "\"TiTle\",\"body\":\"BoDy\",\"title_loc_key\":\"Localized "
              "Title\",\"title_loc_args\":[\"t1\",\"t2\"],\"body_loc_key\":"
              "\"Localized "
              "Body\",\"body_loc_args\":[\"b1\",\"b2\"]},\"data\":{"
              "\"profileName\":"
              "\"My Profile 123\"}}}}")))
      .Times(1);

  EXPECT_CALL(*deliveryTaskCurlAdapter,
              append_header(Eq(5), StrEq("content-type: application/json")))
      .Times(1);

  EXPECT_CALL(*deliveryTaskCurlAdapter,
              append_header(Eq(6), StrEq("content-type: application/json")))
      .Times(1);

  EXPECT_CALL(*deliveryTaskCurlAdapter,
              append_header(Eq(5), StrEq("authorization: bearer xcvbn")))
      .Times(1);

  EXPECT_CALL(*deliveryTaskCurlAdapter,
              append_header(Eq(6), StrEq("authorization: bearer xcooo")))
      .Times(1);

  EXPECT_CALL(*deliveryTaskCurlAdapter,
              append_header(Eq(5), StrEq("apns-topic: com.supla")))
      .Times(1);

  EXPECT_CALL(*deliveryTaskCurlAdapter,
              append_header(Eq(6), StrEq("apns-topic: com.supla")))
      .Times(1);

  EXPECT_CALL(*deliveryTaskCurlAdapter,
              append_header(Eq(5), StrEq("apns-push-type: alert")))
      .Times(1);

  EXPECT_CALL(*deliveryTaskCurlAdapter,
              append_header(Eq(6), StrEq("apns-push-type: alert")))
      .Times(1);

  EXPECT_CALL(*deliveryTaskCurlAdapter,
              append_header(Eq(5), StrEq("apns-priority: 10")))
      .Times(1);

  EXPECT_CALL(*deliveryTaskCurlAdapter,
              append_header(Eq(6), StrEq("apns-priority: 10")))
      .Times(1);

  EXPECT_CALL(
      *deliveryTaskCurlAdapter,
      set_opt_url(Eq(5), StrEq("https://push-apns.supla.org/2568548549")))
      .Times(1);

  EXPECT_CALL(
      *deliveryTaskCurlAdapter,
      set_opt_url(Eq(6),
                  StrEq("https://devel-push-apns.supla.org/ybuabnuf548549")))
      .Times(1);

  EXPECT_CALL(
      *deliveryTaskCurlAdapter,
      set_opt_post_fields(
          Eq(5),
          StrEq("{\"aps\":{\"alert\":{\"title\":\"TiTle\",\"body\":\"BoDy\","
                "\"title-"
                "loc-key\":\"Localized "
                "Title\",\"title-loc-args\":[\"t1\",\"t2\"],\"loc-key\":"
                "\"Localized "
                "Body\",\"loc-args\":[\"b1\",\"b2\"]},\"sound\":\"default\","
                "\"content-available\":1},\"profileName\":\"ABCD\"}")))
      .Times(1);

  EXPECT_CALL(
      *deliveryTaskCurlAdapter,
      set_opt_post_fields(
          Eq(6),
          StrEq("{\"aps\":{\"alert\":{\"title\":\"TiTle\",\"body\":\"BoDy\","
                "\"title-"
                "loc-key\":\"Localized "
                "Title\",\"title-loc-args\":[\"t1\",\"t2\"],\"loc-key\":"
                "\"Localized "
                "Body\",\"loc-args\":[\"b1\",\"b2\"]},\"sound\":\"default\","
                "\"content-available\":1},\"profileName\":\"ABCD\"}")))
      .Times(1);

  shared_ptr<supla_abstract_asynctask> task =
      (new supla_pn_delivery_task(supla_caller(ctClient, 123), 1, queue, pool,
                                  push, provider, &throttling))
          ->start();

  WaitForState(task, supla_asynctask_state::SUCCESS, 1000000);

  EXPECT_TRUE(r1->is_exists());
  EXPECT_TRUE(r2->is_exists());
  EXPECT_TRUE(r3->is_exists());
}

TEST_F(DeliveryTaskTest, protocolVersionGe23) {
  supla_push_notification *push = new supla_push_notification();
  push->set_title("TiTle");
  push->set_body("BoDy");
  push->set_localized_title("Localized Title");
  push->set_localized_body("Localized Body");

  vector<string> targs{"t1", "t2"};
  vector<string> bargs{"b1", "b2"};

  push->set_localized_title_args(targs);
  push->set_localized_body_args(bargs);

  supla_pn_recipient *r1 = new supla_pn_recipient(
      1, 0, false, "0956469ed2650ed09534e4193ef8028f950", "My Profile 123", 23);

  push->get_recipients().add(r1, platform_push_android);

  EXPECT_CALL(
      *deliveryTaskCurlAdapter,
      set_opt_post_fields(
          Eq(0),
          StrEq(
              "{\"message\":{\"token\":\"0956469ed2650ed09534e4193ef8028f950\","
              "\"android\":{\"priority\":\"high\",\"data\":{\"profileName\":"
              "\"My "
              "Profile "
              "123\",\"title\":\"TiTle\",\"body\":\"BoDy\",\"title_loc_key\":"
              "\"Localized "
              "Title\",\"title_loc_arg1\":\"t1\",\"title_loc_arg2\":\"t2\","
              "\"body_loc_key\":\"Localized "
              "Body\",\"body_loc_arg1\":\"b1\",\"body_loc_arg2\":\"b2\"}}}}")))
      .Times(1);

  shared_ptr<supla_abstract_asynctask> task =
      (new supla_pn_delivery_task(supla_caller(ctClient, 123), 1, queue, pool,
                                  push, provider, &throttling))
          ->start();

  WaitForState(task, supla_asynctask_state::SUCCESS, 1000000);
}

TEST_F(DeliveryTaskTest, fcmMessageId) {
  supla_push_notification *push = new supla_push_notification();
  push->set_title("T");
  push->set_body("B");

  supla_pn_recipient *r = new supla_pn_recipient(1, 0, false, "0956469", "", 0);
  push->get_recipients().add(r, platform_push_android);

  EXPECT_CALL(*deliveryTaskCurlAdapter, set_opt_write_data)
      .WillOnce([this](int instance_id, string *request_result) {
        *request_result = "{\"name\":\"projects/abcdfg/messages/MsgId123\"}";
      });

  shared_ptr<supla_abstract_asynctask> task =
      (new supla_pn_delivery_task(supla_caller(ctDevice, 567), 1, queue, pool,
                                  push, provider, &throttling))
          ->start();

  WaitForState(task, supla_asynctask_state::SUCCESS, 1000000);

  EXPECT_EQ(r->get_message_id(), "MsgId123");
}

TEST_F(DeliveryTaskTest, apnsMessageId) {
  supla_push_notification *push = new supla_push_notification();
  push->set_title("T");
  push->set_body("B");

  supla_pn_recipient *r = new supla_pn_recipient(1, 5, false, "0956469", "", 0);
  push->get_recipients().add(r, platform_push_ios);

  EXPECT_CALL(*deliveryTaskCurlAdapter, set_opt_header_data)
      .WillOnce([this](int instance_id, std::list<std::string> *data) {
        data->push_back("apns-id: APNS-ID");
      });

  shared_ptr<supla_abstract_asynctask> task =
      (new supla_pn_delivery_task(supla_caller(ctIPC), 1, queue, pool, push,
                                  provider, &throttling))
          ->start();

  WaitForState(task, supla_asynctask_state::SUCCESS, 1000000);

  EXPECT_EQ(r->get_message_id(), "APNS-ID");
}

TEST_F(DeliveryTaskTest, fcmRecipientDoesNotExist) {
  supla_push_notification *push = new supla_push_notification();
  push->set_title("T");
  push->set_body("B");

  supla_pn_recipient *r = new supla_pn_recipient(1, 0, false, "0956469", "", 0);
  push->get_recipients().add(r, platform_push_android);
  EXPECT_TRUE(r->is_exists());

  EXPECT_CALL(*deliveryTaskCurlAdapter, get_response_code)
      .WillRepeatedly(Return(404));

  EXPECT_CALL(throttling, on_message_not_sent).Times(1);

  shared_ptr<supla_abstract_asynctask> task =
      (new supla_pn_delivery_task(supla_caller(ctIPC), 1, queue, pool, push,
                                  provider, &throttling))
          ->start();

  WaitForState(task, supla_asynctask_state::FAILURE, 1000000);

  EXPECT_FALSE(r->is_exists());
}

TEST_F(DeliveryTaskTest, apnsRecipientDoesNotExist) {
  supla_push_notification *push = new supla_push_notification();
  push->set_title("T");
  push->set_body("B");

  supla_pn_recipient *r = new supla_pn_recipient(1, 5, false, "0956469", "", 0);
  push->get_recipients().add(r, platform_push_ios);
  EXPECT_TRUE(r->is_exists());

  EXPECT_CALL(*deliveryTaskCurlAdapter, get_response_code)
      .WillRepeatedly(Return(400));

  EXPECT_CALL(*deliveryTaskCurlAdapter, set_opt_write_data)
      .WillOnce([this](int instance_id, string *request_result) {
        *request_result = "{\"reason\":\"BadDeviceToken\"}";
      });

  EXPECT_CALL(throttling, on_message_not_sent).Times(1);

  shared_ptr<supla_abstract_asynctask> task =
      (new supla_pn_delivery_task(supla_caller(ctIPC), 1, queue, pool, push,
                                  provider, &throttling))
          ->start();

  WaitForState(task, supla_asynctask_state::FAILURE, 1000000);

  EXPECT_FALSE(r->is_exists());
}

TEST_F(DeliveryTaskTest, apnsEmptyBody) {
  supla_push_notification *push = new supla_push_notification();
  push->set_title("T");

  supla_pn_recipient *r = new supla_pn_recipient(1, 5, false, "0956469", "", 0);
  push->get_recipients().add(r, platform_push_ios);

  EXPECT_CALL(*deliveryTaskCurlAdapter, set_opt_post_fields).Times(0);

  EXPECT_CALL(throttling, on_message_not_sent).Times(1);

  shared_ptr<supla_abstract_asynctask> task =
      (new supla_pn_delivery_task(supla_caller(ctIPC), 1, queue, pool, push,
                                  provider, &throttling))
          ->start();

  WaitForState(task, supla_asynctask_state::FAILURE, 1000000);
}

TEST_F(DeliveryTaskTest, fcmEmptyBody) {
  supla_push_notification *push = new supla_push_notification();
  push->set_title("T");

  supla_pn_recipient *r = new supla_pn_recipient(1, 0, false, "0956469", "", 0);
  push->get_recipients().add(r, platform_push_android);

  EXPECT_CALL(*deliveryTaskCurlAdapter, set_opt_post_fields).Times(0);

  EXPECT_CALL(throttling, on_message_not_sent).Times(1);

  shared_ptr<supla_abstract_asynctask> task =
      (new supla_pn_delivery_task(supla_caller(ctDevice, 567), 1, queue, pool,
                                  push, provider, &throttling))
          ->start();

  WaitForState(task, supla_asynctask_state::FAILURE, 1000000);
}

TEST_F(DeliveryTaskTest, apnsAbortInTitle) {
  supla_push_notification *push = new supla_push_notification();
  push->set_title("{{ abort() }}");
  push->set_body("B");

  supla_pn_recipient *r = new supla_pn_recipient(1, 5, false, "0956469", "", 0);
  push->get_recipients().add(r, platform_push_ios);

  EXPECT_CALL(*deliveryTaskCurlAdapter, set_opt_post_fields).Times(0);

  EXPECT_CALL(throttling, on_message_not_sent).Times(1);

  shared_ptr<supla_abstract_asynctask> task =
      (new supla_pn_delivery_task(supla_caller(ctIPC), 1, queue, pool, push,
                                  provider, &throttling))
          ->start();

  WaitForState(task, supla_asynctask_state::FAILURE, 1000000);
}

TEST_F(DeliveryTaskTest, fcmAbortInTitle) {
  supla_push_notification *push = new supla_push_notification();
  push->set_title("{{ abort() }}");
  push->set_body("B");

  supla_pn_recipient *r = new supla_pn_recipient(1, 0, false, "0956469", "", 0);
  push->get_recipients().add(r, platform_push_android);

  EXPECT_CALL(*deliveryTaskCurlAdapter, set_opt_post_fields).Times(0);

  EXPECT_CALL(throttling, on_message_not_sent).Times(1);

  shared_ptr<supla_abstract_asynctask> task =
      (new supla_pn_delivery_task(supla_caller(ctDevice, 567), 1, queue, pool,
                                  push, provider, &throttling))
          ->start();

  WaitForState(task, supla_asynctask_state::FAILURE, 1000000);
}

TEST_F(DeliveryTaskTest, apnsAbortInLocalizedTitle) {
  supla_push_notification *push = new supla_push_notification();
  push->set_localized_title("{{ abort() }}");
  push->set_body("B");

  supla_pn_recipient *r = new supla_pn_recipient(1, 5, false, "0956469", "", 0);
  push->get_recipients().add(r, platform_push_ios);

  EXPECT_CALL(*deliveryTaskCurlAdapter, set_opt_post_fields).Times(0);

  EXPECT_CALL(throttling, on_message_not_sent).Times(1);

  shared_ptr<supla_abstract_asynctask> task =
      (new supla_pn_delivery_task(supla_caller(ctIPC), 1, queue, pool, push,
                                  provider, &throttling))
          ->start();

  WaitForState(task, supla_asynctask_state::FAILURE, 1000000);
}

TEST_F(DeliveryTaskTest, fcmAbortInLocalizedTitle) {
  supla_push_notification *push = new supla_push_notification();
  push->set_localized_title("{{ abort() }}");
  push->set_body("B");

  supla_pn_recipient *r = new supla_pn_recipient(1, 0, false, "0956469", "", 0);
  push->get_recipients().add(r, platform_push_android);

  EXPECT_CALL(*deliveryTaskCurlAdapter, set_opt_post_fields).Times(0);

  EXPECT_CALL(throttling, on_message_not_sent).Times(1);

  shared_ptr<supla_abstract_asynctask> task =
      (new supla_pn_delivery_task(supla_caller(ctDevice, 567), 1, queue, pool,
                                  push, provider, &throttling))
          ->start();

  WaitForState(task, supla_asynctask_state::FAILURE, 1000000);
}

TEST_F(DeliveryTaskTest, apnsAbortInBody) {
  supla_push_notification *push = new supla_push_notification();
  push->set_title("T");
  push->set_body("{{ abort() }}");

  supla_pn_recipient *r = new supla_pn_recipient(1, 5, false, "0956469", "", 0);
  push->get_recipients().add(r, platform_push_ios);

  EXPECT_CALL(*deliveryTaskCurlAdapter, set_opt_post_fields).Times(0);

  EXPECT_CALL(throttling, on_message_not_sent).Times(1);

  shared_ptr<supla_abstract_asynctask> task =
      (new supla_pn_delivery_task(supla_caller(ctIPC), 1, queue, pool, push,
                                  provider, &throttling))
          ->start();

  WaitForState(task, supla_asynctask_state::FAILURE, 1000000);
}

TEST_F(DeliveryTaskTest, fcmAbortInBody) {
  supla_push_notification *push = new supla_push_notification();
  push->set_title("T");
  push->set_body("{{ abort() }}");

  supla_pn_recipient *r = new supla_pn_recipient(1, 0, false, "0956469", "", 0);
  push->get_recipients().add(r, platform_push_android);

  EXPECT_CALL(*deliveryTaskCurlAdapter, set_opt_post_fields).Times(0);

  EXPECT_CALL(throttling, on_message_not_sent).Times(1);

  shared_ptr<supla_abstract_asynctask> task =
      (new supla_pn_delivery_task(supla_caller(ctDevice, 567), 1, queue, pool,
                                  push, provider, &throttling))
          ->start();

  WaitForState(task, supla_asynctask_state::FAILURE, 1000000);
}

TEST_F(DeliveryTaskTest, apnsAbortInLocalizedBody) {
  supla_push_notification *push = new supla_push_notification();
  push->set_title("T");
  push->set_localized_body("{{ abort() }}");

  supla_pn_recipient *r = new supla_pn_recipient(1, 5, false, "0956469", "", 0);
  push->get_recipients().add(r, platform_push_ios);

  EXPECT_CALL(*deliveryTaskCurlAdapter, set_opt_post_fields).Times(0);

  EXPECT_CALL(throttling, on_message_not_sent).Times(1);

  shared_ptr<supla_abstract_asynctask> task =
      (new supla_pn_delivery_task(supla_caller(ctIPC), 1, queue, pool, push,
                                  provider, &throttling))
          ->start();

  WaitForState(task, supla_asynctask_state::FAILURE, 1000000);
}

TEST_F(DeliveryTaskTest, fcmAbortInLocalizedBody) {
  supla_push_notification *push = new supla_push_notification();
  push->set_title("T");
  push->set_localized_body("{{ abort() }}");

  supla_pn_recipient *r = new supla_pn_recipient(1, 0, false, "0956469", "", 0);
  push->get_recipients().add(r, platform_push_android);

  EXPECT_CALL(*deliveryTaskCurlAdapter, set_opt_post_fields).Times(0);

  EXPECT_CALL(throttling, on_message_not_sent).Times(1);

  shared_ptr<supla_abstract_asynctask> task =
      (new supla_pn_delivery_task(supla_caller(ctDevice, 567), 1, queue, pool,
                                  push, provider, &throttling))
          ->start();

  WaitForState(task, supla_asynctask_state::FAILURE, 1000000);
}

TEST_F(DeliveryTaskTest, fcmGetChannel) {
  supla_push_notification *push = new supla_push_notification();
  push->set_title("TiTle connected: {{ getChannel(123).connected }}");
  push->set_body("BoDy connected: {{ getChannel(123).connected }}");
  push->set_localized_title(
      "Localized Title connected: {{ getChannel(123).connected }}");
  push->set_localized_body(
      "Localized Body connected: {{ getChannel(123).connected }}");

  vector<string> targs{"t1", "t2"};
  vector<string> bargs{"b1", "b2"};

  push->set_localized_title_args(targs);
  push->set_localized_body_args(bargs);

  supla_pn_recipient *r1 = new supla_pn_recipient(
      1, 0, false, "0956469ed2650ed09534e4193ef8028f950", "My Profile 123", 23);

  push->get_recipients().add(r1, platform_push_android);

  EXPECT_CALL(
      *deliveryTaskCurlAdapter,
      set_opt_post_fields(
          Eq(0),
          StrEq(
              "{\"message\":{\"token\":\"0956469ed2650ed09534e4193ef8028f950\","
              "\"android\":{\"priority\":\"high\",\"data\":{\"profileName\":"
              "\"My Profile 123\",\"title\":\"TiTle connected: "
              "false\",\"body\":\"BoDy connected: "
              "false\",\"title_loc_key\":\"Localized Title connected: "
              "false\",\"title_loc_arg1\":\"t1\",\"title_loc_arg2\":\"t2\","
              "\"body_loc_key\":\"Localized Body connected: "
              "false\",\"body_loc_arg1\":\"b1\",\"body_loc_arg2\":\"b2\"}}}}")))
      .Times(1);

  shared_ptr<supla_abstract_asynctask> task =
      (new supla_pn_delivery_task(supla_caller(ctClient, 123), 1, queue, pool,
                                  push, provider, &throttling))
          ->start();

  WaitForState(task, supla_asynctask_state::SUCCESS, 1000000);
}

TEST_F(DeliveryTaskTest, apnsGetChannelWithMockedGetter) {
  supla_push_notification *push = new supla_push_notification();
  push->set_title(
      "TiTle {% set ch = getChannel(5) %}{{ ch.connected }}, {{ ch.value }}, "
      "{{ch.closed }}, {{ ch.open }}");
  push->set_body(
      "BoDy {% set ch = getChannel(5) %}{{ ch.connected }}, {{ ch.value }}, "
      "{{ch.closed }}, {{ ch.open }}");
  push->set_localized_title(
      "Localized Title {% set ch = getChannel(5) %}{{ ch.connected }}, {{ "
      "ch.value }}, {{ch.closed }}, {{ ch.open }}");
  push->set_localized_body(
      "Localized Body {% set ch = getChannel(5) %}{{ ch.connected }}, {{ "
      "ch.value }}, {{ch.closed }}, {{ ch.open }}");

  vector<string> targs{"t1", "t2"};
  vector<string> bargs{"b1", "b2"};

  push->set_localized_title_args(targs);
  push->set_localized_body_args(bargs);

  push->get_recipients().add(
      new supla_pn_recipient(1, 6, true, "ybuabnuf548549", "ABCD", 0),
      platform_push_ios);

  ChannelPropertyGetterMock *getter = new ChannelPropertyGetterMock(1, 0, 0);

  push->set_getter(getter);

  EXPECT_CALL(*getter, _get_value(1, 0, 5, NotNull(), NotNull()))
      .Times(4)
      .WillRepeatedly([](int user_id, int device_id, int channel_id,
                         supla_channel_fragment *fragment,
                         supla_channel_availability_status *status)
                          -> supla_abstract_channel_value * {
        *fragment = supla_channel_fragment(
            device_id, channel_id, 0, SUPLA_CHANNELTYPE_BINARYSENSOR,
            SUPLA_CHANNELFNC_OPENINGSENSOR_GATE, 0, false);
        *status = supla_channel_availability_status(false);
        return new supla_channel_binary_sensor_value(
            SUPLA_CHANNELFNC_OPENINGSENSOR_GATE);
      });

  EXPECT_CALL(
      *deliveryTaskCurlAdapter,
      set_opt_post_fields(
          Eq(6),
          StrEq(
              "{\"aps\":{\"alert\":{\"title\":\"TiTle true, false, false, "
              "true\",\"body\":\"BoDy true, false, false, "
              "true\",\"title-loc-key\":\"Localized Title true, false, false, "
              "true\",\"title-loc-args\":[\"t1\",\"t2\"],\"loc-key\":"
              "\"Localized Body true, false, false, "
              "true\",\"loc-args\":[\"b1\",\"b2\"]},\"sound\":\"default\","
              "\"content-available\":1},\"profileName\":\"ABCD\"}")))
      .Times(1);

  shared_ptr<supla_abstract_asynctask> task =
      (new supla_pn_delivery_task(supla_caller(ctClient, 123), 1, queue, pool,
                                  push, provider, &throttling))
          ->start();

  WaitForState(task, supla_asynctask_state::SUCCESS, 1000000);
}

TEST_F(DeliveryTaskTest, oneNotSent) {
  supla_push_notification *push = new supla_push_notification();
  push->set_title("T");
  push->set_body("B");

  supla_pn_recipient *r1 =
      new supla_pn_recipient(1, 0, false, "0956469", "", 0);
  supla_pn_recipient *r2 =
      new supla_pn_recipient(1, 5, false, "2568548549", "ABCD", 0);

  push->get_recipients().add(r1, platform_push_android);
  push->get_recipients().add(r2, platform_push_ios);

  EXPECT_CALL(*deliveryTaskCurlAdapter, get_response_code(5))
      .WillRepeatedly(Return(404));

  EXPECT_CALL(*deliveryTaskCurlAdapter, get_response_code(0))
      .WillRepeatedly(Return(200));

  EXPECT_CALL(throttling, on_message_not_sent).Times(1);

  shared_ptr<supla_abstract_asynctask> task =
      (new supla_pn_delivery_task(supla_caller(ctIPC), 1, queue, pool, push,
                                  provider, &throttling))
          ->start();

  WaitForState(task, supla_asynctask_state::FAILURE, 1000000);
}
} /* namespace testing */
