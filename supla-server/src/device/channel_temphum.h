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

#ifndef CHANNEL_TEMPHUM_H_
#define CHANNEL_TEMPHUM_H_

#include "proto.h"

class supla_channel_temphum {
 private:
  int ChannelId;
  bool TempAndHumidity;
  double Temperature;
  double Humidity;
  static char tarr_clean(void *ptr);

 public:
  supla_channel_temphum(bool TempAndHumidity, int ChannelId, double Temperature,
                        double Humidity);
  supla_channel_temphum(bool TempAndHumidity, int ChannelId,
                        const char value[SUPLA_CHANNELVALUE_SIZE]);
  supla_channel_temphum(int ChannelId, int Func,
                        const char value[SUPLA_CHANNELVALUE_SIZE]);

  int getChannelId(void);
  bool isTempAndHumidity(void);
  double getTemperature(void);
  double getHumidity(void);
  void setTemperature(double Temperature);
  void setHumidity(double Humidity);
  void toValue(char value[SUPLA_CHANNELVALUE_SIZE]);
  void assignValue(const char value[SUPLA_CHANNELVALUE_SIZE],
                   bool TempAndHumidity);

  static void free(void *tarr);
};

#endif /* CHANNEL_TEMPHUM_H_ */
