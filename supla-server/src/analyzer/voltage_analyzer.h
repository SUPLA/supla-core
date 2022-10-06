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

#include "analyzer/simple_statistics.h"

#ifndef VOLTAGE_ANALYZER_H_
#define VOLTAGE_ANALYZER_H_

class supla_voltage_analyzer : public supla_simple_statiscics {
 private:
  double lower_threshold;
  double upper_threshold;
  int lower_counter;
  int upper_counter;
  struct timeval lower_time;
  struct timeval upper_time;
  int total_sec_above;
  int total_sec_below;
  int max_sec_above;
  int max_sec_below;

 public:
  supla_voltage_analyzer(void);
  virtual ~supla_voltage_analyzer(void);

  void set_lower_threshold(double lower_threshold);
  void set_upper_threshold(double upper_threshold);

  int get_lower_counter(void);
  int get_upper_counter(void);
  int get_total_sec_above(void);
  int get_total_sec_below(void);
  int get_max_sec_above(void);
  int get_max_sec_below(void);

  virtual void update(double value);
  virtual int reset(void);
};

#endif /*VOLTAGE_ANALYZER_H_*/
