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
#ifndef HTTP_THROTTLING_H_
#define HTTP_THROTTLING_H_

#include <sys/time.h>

#include <list>

class supla_http_throttling {
 private:
  typedef struct {
    int channel_id;
    struct timeval last;
    unsigned int counter;
  } item_t;

  std::list<item_t> items;
  void *lck;

 public:
  supla_http_throttling(void);
  virtual ~supla_http_throttling(void);
  int get_delay_time(int channel_id, int func);

  virtual int get_default_delay_time(int func) = 0;
  virtual int get_delay_time_over_threadshold(int func) = 0;
  virtual int get_reset_time_us(int func) = 0;
  virtual unsigned int get_counter_threadshold(int func) = 0;
};

#endif /* HTTP_THROTTLING_H_ */
