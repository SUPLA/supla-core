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

#ifndef CHANNEL_FRAGMENT_H_
#define CHANNEL_FRAGMENT_H_

#include "device/devicechannel.h"

class supla_channel_fragment {
 private:
  int device_id;
  int channel_id;
  unsigned char channel_number;
  int type;
  int function;
  unsigned int flags;
  bool _is_hidden;

 public:
  supla_channel_fragment();
  supla_channel_fragment(int device_id, int channel_id,
                         unsigned char channel_number, int type, int function,
                         unsigned int flags, bool _is_hidden);

  int get_device_id(void);
  int get_channel_id(void);
  unsigned char get_channel_number(void);
  int get_type(void);
  int get_function(void);
  unsigned int get_flags(void);
  void set_function(int function);
  bool is_hidden(void);

  supla_channel_fragment &operator=(supla_device_channel *channel);
};

#endif /* CHANNEL_FRAGMENT_H_ */
