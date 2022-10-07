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

#include <sys/time.h>

#ifndef SIMPLE_STATISTICS_H_
#define SIMPLE_STATISTICS_H_
class supla_simple_statiscics {
 private:
  int channel_id;
  double min;
  double max;
  double avg;
  double sum;
  unsigned int count;
  struct timeval first_update_time;

 public:
  supla_simple_statiscics(void);
  virtual ~supla_simple_statiscics(void);
  void set_channel_id(int channel_id);
  int get_channel_id(void);
  double get_min(void);
  double get_max(void);
  double get_avg(void);
  unsigned int get_sample_count(void);
  virtual void add_sample(double value);
  unsigned int get_total_time_msec(void);
  virtual void reset(void);
};

#endif /*SIMPLE_STATISTICS_H_*/
