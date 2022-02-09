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

#ifndef CHANNEL_ELECTRICITY_MEASUREMENT_H_
#define CHANNEL_ELECTRICITY_MEASUREMENT_H_

#include "proto.h"

class supla_channel_electricity_measurement {
 private:
  TElectricityMeter_ExtendedValue_V2 em_ev;
  int ChannelId;
  void assign(int Param2, const char *TextParam1,
              TElectricityMeter_ExtendedValue_V2 *em_ev);

  static void set_costs(int Param2, const char *TextParam1,
                        TElectricityMeter_ExtendedValue_V2 *em_ev);

  static char emarr_clean(void *ptr);

 public:
  supla_channel_electricity_measurement(
      int ChannelId, TElectricityMeter_ExtendedValue_V2 *em_ev, int Param2,
      const char *TextParam1);

  int getChannelId(void);
  void getMeasurement(TElectricityMeter_ExtendedValue_V2 *em_ev);
  void getCurrency(char currency[4]);

  static bool update_cev(TSC_SuplaChannelExtendedValue *cev, int Param2,
                         const char *TextParam1);
  static void free(void *emarr);
};

#endif /* CHANNEL_ELECTRICITY_MEASUREMENT_H_ */
