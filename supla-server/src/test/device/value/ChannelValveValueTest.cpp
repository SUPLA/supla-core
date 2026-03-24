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

#include "ChannelValveValueTest.h"

#include <string>

#include "device/value/channel_valve_value.h"
#include "proto.h"

using std::map;
using std::string;

namespace testing {

TEST_F(ChannelValveValueTest, voidConstructor) {
  char raw_value1[SUPLA_CHANNELVALUE_SIZE] = {};
  char raw_value2[SUPLA_CHANNELVALUE_SIZE] = {};

  TValve_Value vv1 = {};
  TValve_Value vv2 = {};

  supla_channel_valve_value v(SUPLA_CHANNELFNC_VALVE_OPENCLOSE);
  v.get_raw_value(raw_value2);
  EXPECT_EQ(memcmp(raw_value1, raw_value2, SUPLA_CHANNELVALUE_SIZE), 0);
  v.get_valve_value(&vv2);
  EXPECT_EQ(memcmp(&vv1, &vv2, sizeof(TValve_Value)), 0);
  EXPECT_EQ(memcmp(&vv1, v.get_valve_value(), sizeof(TValve_Value)), 0);
}

TEST_F(ChannelValveValueTest, nonVoidConstructor) {
  TValve_Value vv1 = {};
  TValve_Value vv2 = {};

  vv1.closed_percent = 2;
  vv1.flags = 3;

  supla_channel_valve_value v1(SUPLA_CHANNELFNC_VALVE_OPENCLOSE, &vv1);
  supla_channel_valve_value v2(SUPLA_CHANNELFNC_VALVE_OPENCLOSE,
                               v1.get_valve_value());

  v2.get_valve_value(&vv2);
  EXPECT_EQ(memcmp(&vv1, &vv2, sizeof(TValve_Value)), 0);
}

TEST_F(ChannelValveValueTest, setterAndGetter) {
  supla_channel_valve_value v(SUPLA_CHANNELFNC_VALVE_OPENCLOSE);

  TValve_Value vv1 = {};
  TValve_Value vv2 = {};

  vv1.closed = 20;
  vv1.flags = 10;

  v.set_valve_value(&vv1);
  vv1.closed = 0;

  v.get_valve_value(&vv2);

  EXPECT_EQ(vv2.closed, (unsigned char)20);
  EXPECT_EQ(vv2.flags, (unsigned char)10);
}

TEST_F(ChannelValveValueTest, templateData) {
  supla_channel_valve_value v(SUPLA_CHANNELFNC_VALVE_OPENCLOSE);

  auto d = v.get_template_data();
  EXPECT_EQ(d.size(), 7);
  EXPECT_FALSE(d["flooding"].get<bool>());
  EXPECT_FALSE(d["manually_closed"].get<bool>());
  EXPECT_FALSE(d["motor_problem"].get<bool>());
  EXPECT_FALSE(d["closed"].get<bool>());
  EXPECT_TRUE(d["open"].get<bool>());
  EXPECT_FALSE(d["value"].get<bool>());
  EXPECT_EQ(d["is_closed_manually"].get<std::string>(), "No");

  char raw_value[SUPLA_CHANNELVALUE_SIZE] = {};
  ((TValve_Value*)raw_value)->closed = 1;
  ((TValve_Value*)raw_value)->flags = SUPLA_VALVE_FLAG_MANUALLY_CLOSED;
  v.set_raw_value(raw_value);
  d = v.get_template_data();

  EXPECT_EQ(d.size(), 7);
  EXPECT_FALSE(d["flooding"].get<bool>());
  EXPECT_TRUE(d["manually_closed"].get<bool>());
  EXPECT_FALSE(d["motor_problem"].get<bool>());
  EXPECT_TRUE(d["closed"].get<bool>());
  EXPECT_FALSE(d["open"].get<bool>());
  EXPECT_TRUE(d["value"].get<bool>());
  EXPECT_EQ(d["is_closed_manually"].get<std::string>(), "Yes");

  ((TValve_Value*)raw_value)->flags = SUPLA_VALVE_FLAG_FLOODING;
  v.set_raw_value(raw_value);
  d = v.get_template_data();

  EXPECT_EQ(d.size(), 7);
  EXPECT_TRUE(d["flooding"].get<bool>());
  EXPECT_FALSE(d["manually_closed"].get<bool>());
  EXPECT_FALSE(d["motor_problem"].get<bool>());
  EXPECT_TRUE(d["closed"].get<bool>());
  EXPECT_FALSE(d["open"].get<bool>());
  EXPECT_TRUE(d["value"].get<bool>());
  EXPECT_EQ(d["is_closed_manually"].get<std::string>(), "No");

  ((TValve_Value*)raw_value)->flags = SUPLA_VALVE_FLAG_MOTOR_PROBLEM;
  v.set_raw_value(raw_value);
  d = v.get_template_data();

  EXPECT_EQ(d.size(), 7);
  EXPECT_FALSE(d["flooding"].get<bool>());
  EXPECT_FALSE(d["manually_closed"].get<bool>());
  EXPECT_TRUE(d["motor_problem"].get<bool>());
  EXPECT_TRUE(d["closed"].get<bool>());
  EXPECT_FALSE(d["open"].get<bool>());
  EXPECT_TRUE(d["value"].get<bool>());
  EXPECT_EQ(d["is_closed_manually"].get<std::string>(), "No");

  ((TValve_Value*)raw_value)->flags = SUPLA_VALVE_FLAG_MANUALLY_CLOSED |
                                      SUPLA_VALVE_FLAG_MOTOR_PROBLEM |
                                      SUPLA_VALVE_FLAG_FLOODING;
  v.set_raw_value(raw_value);
  d = v.get_template_data();

  EXPECT_EQ(d.size(), 7);
  EXPECT_TRUE(d["flooding"].get<bool>());
  EXPECT_TRUE(d["manually_closed"].get<bool>());
  EXPECT_TRUE(d["motor_problem"].get<bool>());
  EXPECT_TRUE(d["closed"].get<bool>());
  EXPECT_FALSE(d["open"].get<bool>());
  EXPECT_TRUE(d["value"].get<bool>());
  EXPECT_EQ(d["is_closed_manually"].get<std::string>(), "Yes");
}

TEST_F(ChannelValveValueTest, valvePercentage_templateData) {
  supla_channel_valve_value v(SUPLA_CHANNELFNC_VALVE_PERCENTAGE);
  char raw_value[SUPLA_CHANNELVALUE_SIZE] = {};
  ((TValve_Value*)raw_value)->closed = 45;
  v.set_raw_value(raw_value);

  auto d = v.get_template_data();
  EXPECT_EQ(d.size(), 7);
  EXPECT_FALSE(d["closed"].get<bool>());
  EXPECT_TRUE(d["open"].get<bool>());
  EXPECT_EQ(d["value"].get<int>(), 45);
}

TEST_F(ChannelValveValueTest, getVbtValue) {
  supla_channel_valve_value value(SUPLA_CHANNELFNC_VALVE_OPENCLOSE);
  TValve_Value raw = {};

  double vbt_value = 0;
  EXPECT_TRUE(value.get_vbt_value(var_name_none, &vbt_value));
  EXPECT_EQ(vbt_value, 0);

  EXPECT_TRUE(value.get_vbt_value(var_name_flooding, &vbt_value));
  EXPECT_EQ(vbt_value, 0);

  EXPECT_TRUE(value.get_vbt_value(var_name_manually_closed, &vbt_value));
  EXPECT_EQ(vbt_value, 0);

  raw.closed = 1;
  value.set_valve_value(&raw);
  EXPECT_TRUE(value.get_vbt_value(var_name_none, &vbt_value));
  EXPECT_EQ(vbt_value, 1);

  raw.closed = 0;
  raw.flags = SUPLA_VALVE_FLAG_FLOODING;
  value.set_valve_value(&raw);
  EXPECT_TRUE(value.get_vbt_value(var_name_flooding, &vbt_value));
  EXPECT_EQ(vbt_value, 1);

  raw.flags = SUPLA_VALVE_FLAG_MANUALLY_CLOSED;
  value.set_valve_value(&raw);
  EXPECT_TRUE(value.get_vbt_value(var_name_manually_closed, &vbt_value));
  EXPECT_EQ(vbt_value, 1);
}

}  // namespace testing
