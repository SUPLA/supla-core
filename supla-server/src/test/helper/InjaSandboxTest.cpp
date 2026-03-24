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

#include "InjaSandboxTest.h"

#include <iostream>
#include <nlohmann/json.hpp>
#include <random>
#include <stdexcept>
#include <string>

#include "channel_availability_status.h"
#include "channel_fragment.h"
#include "device/extended_value/channel_em_extended_value.h"
#include "device/extended_value/channel_ic_extended_value.h"
#include "device/value/abstract_channel_value.h"
#include "device/value/channel_binary_sensor_value.h"
#include "device/value/channel_em_value.h"
#include "device/value/channel_ic_value.h"
#include "doubles/device/ChannelPropertyGetterMock.h"
#include "exception/abort_exception.h"
#include "extended_value/abstract_channel_extended_value.h"
#include "gmock/gmock.h"
#include "proto.h"
namespace testing {

using std::runtime_error;

InjaSandboxTest::InjaSandboxTest(void) {}

InjaSandboxTest::~InjaSandboxTest(void) {}

TEST_F(InjaSandboxTest, includeStatementsAreNotAllowed) {
  nlohmann::json data;
  try {
    sandbox.validate_and_render("{% include \"content.html\" %}", data);
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error& e) {
    EXPECT_EQ(std::string("Include statements are not allowed."), e.what());
  }
}

TEST_F(InjaSandboxTest, extendsStatementsAreNotAllowed) {
  nlohmann::json data;
  try {
    sandbox.validate_and_render("{% extends \"content.html\" %}", data);
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error& e) {
    EXPECT_EQ(std::string("Extends statements are not allowed."), e.what());
  }
}

TEST_F(InjaSandboxTest, forLoopssAreNotAllowed) {
  nlohmann::json data;
  try {
    sandbox.validate_and_render(
        "{% for i in range(4) %}{{ loop.index1 }}{% endfor %}", data);
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error& e) {
    EXPECT_EQ(std::string("For loops are not allowed."), e.what());
  }
}

TEST_F(InjaSandboxTest, tooManyControlBlocksInTemplate) {
  nlohmann::json data;
  try {
    sandbox.validate_and_render(
        "{% {% {% {% {% {% {% {% {% {% {% {% {% {% {% {% {% {% {% {% {% ",
        data);
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error& e) {
    EXPECT_EQ(std::string("Too many control blocks in template."), e.what());
  }
}

TEST_F(InjaSandboxTest, exceededOutputSize) {
  nlohmann::json data;

  std::string text;
  text.reserve(1025);

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(0, 9);

  for (std::size_t i = 0; i < 1025; ++i) {
    text += static_cast<char>('0' + dis(gen));
  }

  data["text"] = text;
  try {
    sandbox.validate_and_render("{{ text }}", data);
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error& e) {
    EXPECT_EQ(
        std::string("Template execution exceeded allowed output size (1024)."),
        e.what());
  }
}

TEST_F(InjaSandboxTest, abortFunction) {
  nlohmann::json data;
  std::string tmpl = "{{ abort() }}";

  try {
    sandbox.validate_and_render(tmpl, data);
    FAIL() << "Expected exception";
  } catch (const std::exception& e) {
    EXPECT_EQ(
        std::string(
            "[inja.exception.parser_error] (at 1:10) unknown function abort"),
        e.what());
  }

  try {
    sandbox.register_abort_function();
    sandbox.validate_and_render(tmpl, data);
    FAIL() << "Expected abort_exception";
  } catch (const abort_exception& e) {
    EXPECT_EQ(std::string("Render aborted."), e.what());
  }
}

TEST_F(InjaSandboxTest, getChannelFunction_NotRegistered) {
  nlohmann::json data;
  std::string tmpl = "{% set ch = getChannel(5) %}";

  try {
    sandbox.validate_and_render(tmpl, data);
    FAIL() << "Expected exception";
  } catch (const std::exception& e) {
    EXPECT_EQ(std::string("[inja.exception.parser_error] (at 1:25) unknown "
                          "function getChannel"),
              e.what());
  }
}

TEST_F(InjaSandboxTest, getChannelFunction_Value) {
  nlohmann::json data;
  std::string tmpl =
      "{% set ch = getChannel(5) %}{{ ch.connected }}, {{ ch.value }}, {{ "
      "ch.closed }}, {{ ch.open }}";

  ChannelPropertyGetterMock getter(155, 0, 0);

  EXPECT_CALL(getter, _get_value(155, 0, 5, NotNull(), NotNull()))
      .WillOnce([](int user_id, int device_id, int channel_id,
                   supla_channel_fragment* fragment,
                   supla_channel_availability_status* status)
                    -> supla_abstract_channel_value* {
        *fragment = supla_channel_fragment(
            device_id, channel_id, 0, SUPLA_CHANNELTYPE_BINARYSENSOR,
            SUPLA_CHANNELFNC_OPENINGSENSOR_GATE, 0, false);
        *status = supla_channel_availability_status(false);
        return new supla_channel_binary_sensor_value(
            SUPLA_CHANNELFNC_OPENINGSENSOR_GATE);
      });

  sandbox.register_get_channel_function(&getter);

  EXPECT_EQ(sandbox.validate_and_render(tmpl, data),
            "true, false, false, true");
}

TEST_F(InjaSandboxTest, getChannelFunction_ExtendedValue_EM) {
  nlohmann::json data;
  std::string tmpl =
      "{% set ch = getChannel(50) %}{{ ch.connected }}, {{ round(ch.fae1, 1) "
      "}}, {{  round(ch.fae2, 1) }}, {{  round(ch.fae3, 1) }}";

  ChannelPropertyGetterMock getter(155, 0, 0);

  EXPECT_CALL(getter, _get_value(155, 0, 50, NotNull(), NotNull()))
      .WillOnce([](int user_id, int device_id, int channel_id,
                   supla_channel_fragment* fragment,
                   supla_channel_availability_status* status)
                    -> supla_abstract_channel_value* {
        *fragment = supla_channel_fragment(
            device_id, channel_id, 0, SUPLA_CHANNELTYPE_ELECTRICITY_METER,
            SUPLA_CHANNELFNC_ELECTRICITY_METER, 0, false);
        *status = supla_channel_availability_status(false);
        TElectricityMeter_Value raw = {};
        return new supla_channel_em_value(&raw);
      });

  EXPECT_CALL(getter, _get_extended_value(155, 0, 50))
      .WillOnce([](int user_id, int device_id,
                   int channel_id) -> supla_abstract_channel_extended_value* {
        TElectricityMeter_ExtendedValue_V3 raw = {};
        raw.total_forward_active_energy[0] = 100000;
        raw.total_forward_active_energy[1] = 200000;
        raw.total_forward_active_energy[2] = 300000;
        return new supla_channel_em_extended_value(&raw, "", 0);
      });

  sandbox.register_get_channel_function(&getter);

  EXPECT_EQ(sandbox.validate_and_render(tmpl, data), "true, 1.0, 2.0, 3.0");
}

} /* namespace testing */
