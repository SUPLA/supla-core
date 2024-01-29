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

#include "device/value/channel_general_purpose_measurement_value.h"
#include "test/analyzer/GeneralPurposeMeasurementAnaluzerTest.h"

namespace testing {

TEST_F(GeneralPurposeMeasurementAnalyzerTest, addSample) {
  EXPECT_EQ(analyzer.get_sample_count(), 0);
  supla_channel_general_purpose_measurement_value value;
  value.set_value(123.45);
  analyzer.add_sample(&value);
  value.set_value(56.78);
  analyzer.add_sample(&value);
  EXPECT_EQ(analyzer.get_sample_count(), 2);
  EXPECT_EQ(analyzer.get_first(), 123.45);
  EXPECT_EQ(analyzer.get_last(), 56.78);
}

TEST_F(GeneralPurposeMeasurementAnalyzerTest, reset) {
  supla_channel_general_purpose_measurement_value value;
  value.set_value(1);
  analyzer.add_sample(&value);
  EXPECT_EQ(analyzer.get_sample_count(), 1);
  analyzer.reset();
  EXPECT_EQ(analyzer.get_sample_count(), 0);
}

TEST_F(GeneralPurposeMeasurementAnalyzerTest, anyDataForLogger) {
  EXPECT_FALSE(analyzer.is_any_data_for_logging_purpose());
  supla_channel_general_purpose_measurement_value value;
  value.set_value(1);
  analyzer.add_sample(&value);
  EXPECT_TRUE(analyzer.is_any_data_for_logging_purpose());
}

TEST_F(GeneralPurposeMeasurementAnalyzerTest, copy) {
  analyzer.set_channel_id(456);
  supla_channel_general_purpose_measurement_value value;
  value.set_value(567);
  analyzer.add_sample(&value);

  supla_abstract_data_analyzer *c = analyzer.copy();
  ASSERT_TRUE(c != nullptr);

  supla_general_purpose_measurement_analyzer *mac =
      dynamic_cast<supla_general_purpose_measurement_analyzer *>(c);
  EXPECT_TRUE(mac != nullptr);

  if (mac) {
    EXPECT_EQ(mac->get_channel_id(), analyzer.get_channel_id());
    EXPECT_EQ(mac->get_sample_count(), analyzer.get_sample_count());
    EXPECT_EQ(mac->get_first(), analyzer.get_first());
  }

  delete c;
}

}  // namespace testing
