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

#include <sys/time.h>

#include "datalogger/impulse_logger.h"
#include "datalogger/total_energy_logger.h"
#include "gtest/gtest.h"  // NOLINT

namespace testing {
namespace {

class ImpulseLoggerUnderTest : public supla_impulse_logger {
 public:
  using supla_impulse_logger::task_interval_aligned;
  using supla_impulse_logger::task_interval_sec;
};

class TotalEnergyLoggerUnderTest : public supla_total_energy_logger {
 public:
  using supla_total_energy_logger::task_interval_aligned;
  using supla_total_energy_logger::task_interval_sec;
};

TEST(LoggerScheduleTest, impulseLoggerRunsEveryQuarterHour) {
  ImpulseLoggerUnderTest logger;

  EXPECT_EQ(logger.task_interval_sec(), 900);
  EXPECT_TRUE(logger.task_interval_aligned());
}

TEST(LoggerScheduleTest, totalEnergyLoggerRunsEveryQuarterHour) {
  TotalEnergyLoggerUnderTest logger;

  EXPECT_EQ(logger.task_interval_sec(), 900);
  EXPECT_TRUE(logger.task_interval_aligned());
}

TEST(LoggerScheduleTest, alignedLoggerWaitsForNextIntervalBoundary) {
  ImpulseLoggerUnderTest logger;
  struct timeval now = {};
  gettimeofday(&now, nullptr);

  time_t next_quarter = (now.tv_sec / 900 + 1) * 900;
  now.tv_sec = next_quarter - 1;
  EXPECT_FALSE(logger.is_it_time(&now));

  now.tv_sec = next_quarter;
  EXPECT_TRUE(logger.is_it_time(&now));
}

}  // namespace
}  // namespace testing
