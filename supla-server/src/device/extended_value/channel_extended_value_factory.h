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

#ifndef CHANNEL_EXTENDED_VALUE_FACTORY_H_
#define CHANNEL_EXTENDED_VALUE_FACTORY_H_

#include "device/extended_value/channel_extended_value.h"
#include "user/user.h"

class supla_channel_extended_value_factory {
 public:
  supla_channel_extended_value_factory(void);
  static supla_channel_extended_value *new_value(TSuplaChannelExtendedValue *ev,
                                                 const char *text_param1,
                                                 int param2, supla_user *user);

  static supla_channel_extended_value *new_value(
      TSuplaChannelExtendedValue *ev);
};

#endif /*CHANNEL_EXTENDED_VALUE_FACTORY_H_*/
