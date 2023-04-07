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
#ifndef ALEXA_THROTTLING_H_
#define ALEXA_THROTTLING_H_

#include "http/http_throttling.h"

class supla_alexa_throttling : public supla_http_throttling {
 private:
  static supla_alexa_throttling instance;

 public:
  supla_alexa_throttling(void);
  virtual ~supla_alexa_throttling(void);

  static supla_alexa_throttling *get_instance(void);

  virtual int get_default_delay_time(int func);
  virtual int get_delay_time_over_threadshold(int func);
  virtual int get_reset_time_us(int func);
  virtual unsigned int get_counter_threadshold(int func);
};

#endif /* ALEXA_THROTTLING_H_ */
