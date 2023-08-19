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

#include "ActivePeriodTest.h"

#include "log.h"

namespace testing {

const char ActivePeriodTest::tz[] = "Europe/Warsaw";

ActivePeriodTest::ActivePeriodTest(void) {
  lat = 0;
  lng = 0;
}

ActivePeriodTest::~ActivePeriodTest(void) {}

void ActivePeriodTest::SetUp() {
  Test::SetUp();

  std::time_t timestamp = 1692196976;

  ON_CALL(p, get_current_point_in_time)
      .WillByDefault(Return(std::chrono::time_point<std::chrono::system_clock>(
          std::chrono::seconds(timestamp))));
}

TEST_F(ActivePeriodTest, defaultSettings) {
  EXPECT_TRUE(p.is_now_active(tz, lat, lng));
}

TEST_F(ActivePeriodTest, activeFrom) {
  p.set_active_from(1692196977);
  EXPECT_FALSE(p.is_now_active(tz, lat, lng));

  p.set_active_from(5000000000);
  EXPECT_FALSE(p.is_now_active(tz, lat, lng));

  p.set_active_from(1692196976);
  EXPECT_TRUE(p.is_now_active(tz, lat, lng));

  p.set_active_from(1);
  EXPECT_TRUE(p.is_now_active(tz, lat, lng));
}

TEST_F(ActivePeriodTest, activeTo) {
  p.set_active_to(1692196975);
  EXPECT_FALSE(p.is_now_active(tz, lat, lng));

  p.set_active_to(1);
  EXPECT_FALSE(p.is_now_active(tz, lat, lng));

  p.set_active_to(1692196977);
  EXPECT_TRUE(p.is_now_active(tz, lat, lng));

  p.set_active_to(5000000000);
  EXPECT_TRUE(p.is_now_active(tz, lat, lng));
}

TEST_F(ActivePeriodTest, allActiveHours) {
  p.set_active_hours(
      ",10,11,12,13,14,15,16,17,18,19,110,111,112,113,114,115,116,117,118,119,"
      "120,121,122,123,20,21,22,23,24,25,26,27,28,29,210,211,212,213,214,215,"
      "216,217,218,219,220,221,222,223,30,31,32,33,34,35,36,37,38,39,310,311,"
      "312,313,314,315,316,317,318,319,320,321,322,323,40,41,42,43,44,45,46,47,"
      "48,49,410,411,412,413,414,415,416,417,418,419,420,421,422,423,50,51,52,"
      "53,54,55,56,57,58,59,510,511,512,513,514,515,516,517,518,519,520,521,"
      "522,523,60,61,62,63,64,65,66,67,68,69,610,611,612,613,614,615,616,617,"
      "618,619,620,621,622,623,70,71,72,73,74,75,76,77,78,79,710,711,712,713,"
      "714,715,716,717,718,719,720,721,722,723,");

  std::time_t timestamp = 1692196976;

  EXPECT_CALL(p, get_current_point_in_time).WillRepeatedly([&]() {
    timestamp += 3600;
    return std::chrono::time_point<std::chrono::system_clock>(
        std::chrono::seconds(timestamp));
  });

  for (int d = 0; d < 30; d++) {
    for (int h = 0; h < 24; h++) {
      EXPECT_TRUE(p.is_now_active(tz, lat, lng));
    }
  }
}

TEST_F(ActivePeriodTest, selectedActiveHours) {
  p.set_active_hours(
      ",10,11,12,13,14,15,16,17,18,19,110,111,112,113,114,115,116,117,118,119,"
      "120,121,122,123,20,21,22,23,24,25,26,27,28,29,210,211,212,213,214,215,"
      "216,217,218,219,220,221,222,223,30,31,32,33,34,35,319,320,321,322,323,"
      "40,41,42,43,44,45,419,420,421,422,423,50,51,52,53,54,55,519,520,521,522,"
      "523,60,61,62,63,64,65,66,67,68,69,610,611,612,613,614,615,616,617,618,"
      "619,620,621,622,623,70,71,72,73,74,75,76,77,78,79,710,711,712,713,714,"
      "715,716,717,718,719,720,721,722,723,");

  std::time_t timestamp = 1691964000;

  EXPECT_CALL(p, get_current_point_in_time).WillRepeatedly([&]() {
    auto result = std::chrono::time_point<std::chrono::system_clock>(
        std::chrono::seconds(timestamp));
    timestamp += 3600;
    return result;
  });

  for (int d = 1; d <= 7; d++) {
    for (int h = 0; h < 24; h++) {
      if (h >= 6 && h <= 18 && d >= 3 && d <= 5) {
        ASSERT_FALSE(p.is_now_active(tz, lat, lng));
      } else {
        ASSERT_TRUE(p.is_now_active(tz, lat, lng));
      }
    }
  }
}

TEST_F(ActivePeriodTest, afterSunsetOrBeforeSunrise) {
  // Sunrise + 20 = 06:20:57
  // Sunset - 10 = 17:57:43

  // 1692144000 - 00:00:00
  // 1692166856 - 06:20:56
  // 1692166857 - 06:20:57 *
  // 1692166858 - 06:20:58
  // 1692208662 - 17:57:42
  // 1692208663 - 17:57:43 *
  // 1692208664 - 17:57:44
  // 1692230399 - 23:59:59

  std::time_t timestamp[] = {1692144000, 1692166856, 1692166857, 1692166858,
                             1692208662, 1692208663, 1692208664, 1692230399};

  size_t n = 0;

  EXPECT_CALL(p, get_current_point_in_time).WillRepeatedly([&]() {
    auto result = std::chrono::time_point<std::chrono::system_clock>(
        std::chrono::seconds(timestamp[n]));
    n++;
    return result;
  });

  p.set_astro_conditions(
      "[[{\"afterSunset\": -10}], [{\"beforeSunrise\": 20}]]");

  ASSERT_TRUE(p.is_now_active(tz, lat, lng));
  ASSERT_TRUE(p.is_now_active(tz, lat, lng));
  ASSERT_FALSE(p.is_now_active(tz, lat, lng));
  ASSERT_FALSE(p.is_now_active(tz, lat, lng));
  ASSERT_FALSE(p.is_now_active(tz, lat, lng));
  ASSERT_FALSE(p.is_now_active(tz, lat, lng));
  ASSERT_TRUE(p.is_now_active(tz, lat, lng));
  ASSERT_TRUE(p.is_now_active(tz, lat, lng));
}

TEST_F(ActivePeriodTest, beforeSunriseAndAfterSunset) {
  // Sunrise + 20 = 06:20:57
  // Sunset - 10 = 17:57:43

  // 1692144000 - 00:00:00
  // 1692166856 - 06:20:56
  // 1692166857 - 06:20:57 *
  // 1692166858 - 06:20:58
  // 1692208662 - 17:57:42
  // 1692208663 - 17:57:43 *
  // 1692208664 - 17:57:44
  // 1692230399 - 23:59:59

  std::time_t timestamp[] = {1692144000, 1692166856, 1692166857, 1692166858,
                             1692208662, 1692208663, 1692208664, 1692230399};

  size_t n = 0;

  EXPECT_CALL(p, get_current_point_in_time).WillRepeatedly([&]() {
    auto result = std::chrono::time_point<std::chrono::system_clock>(
        std::chrono::seconds(timestamp[n]));
    n++;
    return result;
  });

  p.set_astro_conditions("[[{\"beforeSunset\": -10}, {\"afterSunrise\": 20}]]");

  ASSERT_FALSE(p.is_now_active(tz, lat, lng));
  ASSERT_FALSE(p.is_now_active(tz, lat, lng));
  ASSERT_FALSE(p.is_now_active(tz, lat, lng));
  ASSERT_TRUE(p.is_now_active(tz, lat, lng));
  ASSERT_TRUE(p.is_now_active(tz, lat, lng));
  ASSERT_FALSE(p.is_now_active(tz, lat, lng));
  ASSERT_FALSE(p.is_now_active(tz, lat, lng));
  ASSERT_FALSE(p.is_now_active(tz, lat, lng));
}

TEST_F(ActivePeriodTest, beforeSunrise) {
  // Sunrise + 20 = 06:20:57

  // 1692144000 - 00:00:00
  // 1692166856 - 06:20:56
  // 1692166857 - 06:20:57 *
  // 1692166858 - 06:20:58
  // 1692230399 - 23:59:59

  std::time_t timestamp[] = {1692144000, 1692166856, 1692166857, 1692166858,
                             1692230399};

  size_t n = 0;

  EXPECT_CALL(p, get_current_point_in_time).WillRepeatedly([&]() {
    auto result = std::chrono::time_point<std::chrono::system_clock>(
        std::chrono::seconds(timestamp[n]));
    n++;
    return result;
  });

  p.set_astro_conditions("[[{\"beforeSunrise\": 20}]]");

  ASSERT_TRUE(p.is_now_active(tz, lat, lng));
  ASSERT_TRUE(p.is_now_active(tz, lat, lng));
  ASSERT_FALSE(p.is_now_active(tz, lat, lng));
  ASSERT_FALSE(p.is_now_active(tz, lat, lng));
  ASSERT_FALSE(p.is_now_active(tz, lat, lng));
}

TEST_F(ActivePeriodTest, afterSunrise) {
  // Sunrise + 20 = 06:20:57

  // 1692144000 - 00:00:00
  // 1692166856 - 06:20:56
  // 1692166857 - 06:20:57 *
  // 1692166858 - 06:20:58
  // 1692230399 - 23:59:59

  std::time_t timestamp[] = {1692144000, 1692166856, 1692166857, 1692166858,
                             1692230399};

  size_t n = 0;

  EXPECT_CALL(p, get_current_point_in_time).WillRepeatedly([&]() {
    auto result = std::chrono::time_point<std::chrono::system_clock>(
        std::chrono::seconds(timestamp[n]));
    n++;
    return result;
  });

  p.set_astro_conditions("[[{\"afterSunrise\": 20}]]");

  ASSERT_FALSE(p.is_now_active(tz, lat, lng));
  ASSERT_FALSE(p.is_now_active(tz, lat, lng));
  ASSERT_FALSE(p.is_now_active(tz, lat, lng));
  ASSERT_TRUE(p.is_now_active(tz, lat, lng));
  ASSERT_TRUE(p.is_now_active(tz, lat, lng));
}

TEST_F(ActivePeriodTest, beforeSunset) {
  // Sunset - 10 = 17:57:43

  // 1692144000 - 00:00:00
  // 1692208662 - 17:57:42
  // 1692208663 - 17:57:43 *
  // 1692208664 - 17:57:44
  // 1692230399 - 23:59:59

  std::time_t timestamp[] = {1692144000, 1692208662, 1692208663, 1692208664,
                             1692230399};

  size_t n = 0;

  EXPECT_CALL(p, get_current_point_in_time).WillRepeatedly([&]() {
    auto result = std::chrono::time_point<std::chrono::system_clock>(
        std::chrono::seconds(timestamp[n]));
    n++;
    return result;
  });

  p.set_astro_conditions("[[{\"beforeSunset\": -10}]]");

  ASSERT_TRUE(p.is_now_active(tz, lat, lng));
  ASSERT_TRUE(p.is_now_active(tz, lat, lng));
  ASSERT_FALSE(p.is_now_active(tz, lat, lng));
  ASSERT_FALSE(p.is_now_active(tz, lat, lng));
  ASSERT_FALSE(p.is_now_active(tz, lat, lng));
}

TEST_F(ActivePeriodTest, afterSunset) {
  // Sunset - 10 = 17:55
  // Sunset - 10 = 17:57:43
  // Sunset - 10 = 17:57:55

  // 1692122276 - 17:57:56 (day before)
  // 1692144000 - 00:00:00
  // 1692208662 - 17:57:42
  // 1692208663 - 17:57:43 *
  // 1692208664 - 17:57:44
  // 1692230399 - 23:59:59
  // 1692122275 - 17:57:55 * (next day)
  // 1692122276 - 17:57:56 (next day)

  std::time_t timestamp[] = {1692122276, 1692144000, 1692208662, 1692208663,
                             1692208664, 1692230399, 1692122275, 1692122276};

  size_t n = 0;

  EXPECT_CALL(p, get_current_point_in_time).WillRepeatedly([&]() {
    auto result = std::chrono::time_point<std::chrono::system_clock>(
        std::chrono::seconds(timestamp[n]));
    n++;
    return result;
  });

  p.set_astro_conditions("[[{\"afterSunset\": -10}]]");

  ASSERT_TRUE(p.is_now_active(tz, lat, lng));
  ASSERT_FALSE(p.is_now_active(tz, lat, lng));
  ASSERT_FALSE(p.is_now_active(tz, lat, lng));
  ASSERT_FALSE(p.is_now_active(tz, lat, lng));
  ASSERT_TRUE(p.is_now_active(tz, lat, lng));
  ASSERT_TRUE(p.is_now_active(tz, lat, lng));
  ASSERT_FALSE(p.is_now_active(tz, lat, lng));
  ASSERT_TRUE(p.is_now_active(tz, lat, lng));
}

TEST_F(ActivePeriodTest, afterSunset_WarsawCoordinates) {
  lat = 52.25;
  lng = 21.00;

  // Sunset - 10 = 17:51:20
  // Sunset - 10 = 17:49:18
  // Sunset - 10 = 17:57:55

  // 1692122585 - 17:53:05 (day before)
  // 1692144000 - 00:00:00
  // 1692208138 - 17:48:58
  // 1692208158 - 17:49:18 *
  // 1692208160 - 17:49:20
  // 1692230399 - 23:59:59
  // 1692294434 - 17:47:14 * (next day)
  // 1692294440 - 17:47:20 (next day)

  std::time_t timestamp[] = {1692122585, 1692144000, 1692208138, 1692208158,
                             1692208160, 1692230399, 1692294434, 1692294440};

  size_t n = 0;

  EXPECT_CALL(p, get_current_point_in_time).WillRepeatedly([&]() {
    auto result = std::chrono::time_point<std::chrono::system_clock>(
        std::chrono::seconds(timestamp[n]));
    n++;
    return result;
  });

  p.set_astro_conditions("[[{\"afterSunset\": -10}]]");

  ASSERT_TRUE(p.is_now_active(tz, lat, lng));
  ASSERT_FALSE(p.is_now_active(tz, lat, lng));
  ASSERT_FALSE(p.is_now_active(tz, lat, lng));
  ASSERT_FALSE(p.is_now_active(tz, lat, lng));
  ASSERT_TRUE(p.is_now_active(tz, lat, lng));
  ASSERT_TRUE(p.is_now_active(tz, lat, lng));
  ASSERT_FALSE(p.is_now_active(tz, lat, lng));
  ASSERT_TRUE(p.is_now_active(tz, lat, lng));
}

TEST_F(ActivePeriodTest, compare) {
  ActivePeriodMock p2;
  EXPECT_TRUE(p == p2);

  p2.set_active_from(1);
  EXPECT_FALSE(p == p2);

  p.set_active_from(1);
  EXPECT_TRUE(p == p2);

  p.set_active_to(2);
  EXPECT_FALSE(p == p2);

  p2.set_active_to(2);
  EXPECT_TRUE(p == p2);

  p.set_active_hours(",10,11,");
  EXPECT_FALSE(p == p2);

  p2.set_active_hours(",10,11,222,");
  EXPECT_FALSE(p == p2);

  p.set_active_hours(",10,11,222,");
  EXPECT_TRUE(p == p2);

  p.set_astro_conditions(
      "[[{\"afterSunset\": -10}], [{\"beforeSunrise\": 20}]]");

  EXPECT_FALSE(p == p2);

  p2.set_astro_conditions(
      "[[{\"afterSunset\": -10}], [{\"beforeSunrise\": 21}]]");

  EXPECT_FALSE(p == p2);

  p2.set_astro_conditions(
      "[[{\"afterSunset\": -11}], [{\"beforeSunrise\": 20}]]");

  EXPECT_FALSE(p == p2);

  p2.set_astro_conditions(
      "[[{\"afterSunset\": -10}, {\"beforeSunrise\": 20}]]");

  EXPECT_FALSE(p == p2);

  p2.set_astro_conditions(
      "[[{\"afterSunset\": -10}], [{\"beforeSunrise\": 20}]]");

  EXPECT_TRUE(p == p2);

  p.set_astro_conditions("[[{\"afterSunset\": -10}, {\"beforeSunrise\": 20}]]");

  EXPECT_FALSE(p == p2);

  p2.set_astro_conditions(
      "[[{\"afterSunset\": -10}, {\"beforeSunrise\": 20}]]");

  EXPECT_TRUE(p == p2);
}

} /* namespace testing */
