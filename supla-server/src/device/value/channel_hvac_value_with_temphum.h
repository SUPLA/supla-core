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

#ifndef CHANNEL_HVAC_VALUE_WITH_TEMP_HUM_H_
#define CHANNEL_HVAC_VALUE_WITH_TEMP_HUM_H_

#include "device/abstract_channel_property_getter.h"
#include "device/value/channel_hvac_value.h"

class supla_channel_hvac_value_with_temphum : public supla_channel_hvac_value {
 private:
  short temperature;
  short humidity;

 public:
  supla_channel_hvac_value_with_temphum(void);
  explicit supla_channel_hvac_value_with_temphum(
      supla_channel_hvac_value *value);
  virtual ~supla_channel_hvac_value_with_temphum(void);

  // !These methods do not affect raw_value!
  void set_temperature(short temperature);
  short get_temperature(void);
  void set_humidity(short humidity);
  short get_humidity(void);
  // ---------------------------------------

  static void extend(supla_channel_value **value,
                     supla_channel_fragment *fragment,
                     supla_abstract_channel_property_getter *getter);
};

#endif /*CHANNEL_HVAC_VALUE_WITH_TEMP_HUM_H_*/
