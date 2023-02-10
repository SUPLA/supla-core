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

#ifndef CHANNEL_TEMPHUM_VALUE_H_
#define CHANNEL_TEMPHUM_VALUE_H_

#include "device/channel_value.h"

class supla_channel_temphum_value : public supla_channel_value {
 private:
  bool with_humidity;

 public:
  supla_channel_temphum_value(bool with_humidity,
                              char native_value[SUPLA_CHANNELVALUE_SIZE]);
  supla_channel_temphum_value(bool with_humidity, double temperature,
                              double humidity);

  double get_temperature(void);
  double get_humidity(void);
  void set_temperature(double temperature);
  void set_humidity(double humidity);
  static bool significant_change(supla_channel_temphum_value *old_val,
                                 supla_channel_temphum_value *new_val);
};

#endif /* CHANNEL_TEMPHUM_VALUE_H_ */
