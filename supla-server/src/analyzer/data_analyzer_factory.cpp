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

#include "data_analyzer_factory.h"

#include "analyzer/electricity_analyzer.h"
#include "analyzer/general_purpose_measurement_analyzer.h"

supla_data_analyzer_factory::supla_data_analyzer_factory() {}

supla_data_analyzer_factory::~supla_data_analyzer_factory(void) {}

// static
supla_abstract_data_analyzer *supla_data_analyzer_factory::new_analyzer(
    int channel_id, int func) {
  switch (func) {
    case SUPLA_CHANNELFNC_ELECTRICITY_METER:
      return new supla_electricity_analyzer(channel_id);
    case SUPLA_CHANNELFNC_GENERAL_PURPOSE_MEASUREMENT:
      return new supla_general_purpose_measurement_analyzer(channel_id);
  }

  return nullptr;
}
