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

#include "analyzer/abstract_data_analyzer.h"
#include "analyzer/simple_statistics.h"

#ifndef GENERAL_PURPOSE_MEASUREMENT_ANALYZER_H_
#define GENERAL_PURPOSE_MEASUREMENT_ANALYZER_H_

class supla_general_purpose_measurement_analyzer
    : public supla_simple_statiscics,
      public supla_abstract_data_analyzer {
 private:
 public:
  supla_general_purpose_measurement_analyzer(void);
  explicit supla_general_purpose_measurement_analyzer(int channel_id);
  virtual ~supla_general_purpose_measurement_analyzer(void);

  virtual void add_sample(supla_channel_value *value,
                          supla_json_config *config);
  virtual void add_sample(int channel_flags, supla_json_config *config,
                          supla_channel_extended_value *extended_value);

  virtual void reset(void);
  virtual bool is_any_data_for_logging_purpose(void);
  virtual supla_abstract_data_analyzer *copy(void);  // NOLINT
};

#endif /*GENERAL_PURPOSE_MEASUREMENT_ANALYZER_H_*/
