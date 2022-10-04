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

#ifndef CHANNEL_IC_MEASUREMENT_H_
#define CHANNEL_IC_MEASUREMENT_H_

#include "proto.h"

class supla_channel_ic_measurement {
 private:
  int ChannelId;

  _supla_int_t totalCost;
  _supla_int_t pricePerUnit;
  char currency[4];
  char customUnit[9];
  _supla_int_t impulsesPerUnit;
  unsigned _supla_int64_t counter;
  _supla_int64_t calculatedValue;

 public:
  supla_channel_ic_measurement(int ChannelId, int Func,
                               TDS_ImpulseCounter_Value *ic_val,
                               const char *TextParam1, const char *TextParam2,
                               int Param2, int Param3);

  int getChannelId(void);
  _supla_int_t getTotalCost(void);
  _supla_int_t getPricePerUnit(void);
  const char *getCurrency(void);
  const char *getCustomUnit(void);
  _supla_int_t getImpulsesPerUnit(void);
  unsigned _supla_int64_t getCounter(void);
  unsigned _supla_int64_t getCalculatedValue(void);

  static void set_default_unit(int Func, char unit[9]);
  static bool update_cev(TSuplaChannelExtendedValue *ev, int Func, int Param2,
                         int Param3, const char *TextParam1,
                         const char *TextParam2);

  static double get_calculated_d(_supla_int_t impulses_per_unit,
                                 unsigned _supla_int64_t counter);
  static _supla_int64_t get_calculated_i(_supla_int_t impulses_per_unit,
                                         unsigned _supla_int64_t counter);
  static void get_cost_and_currency(const char *TextParam1, int Param2,
                                    char currency[3], _supla_int_t *total_cost,
                                    _supla_int_t *price_per_unit, double count);
};

#endif /* CHANNEL_IC_MEASUREMENT_H_ */
