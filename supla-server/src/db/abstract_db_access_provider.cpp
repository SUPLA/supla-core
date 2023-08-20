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

#include "abstract_db_access_provider.h"

supla_abstract_db_access_provider::supla_abstract_db_access_provider(void) {}

supla_abstract_db_access_provider::~supla_abstract_db_access_provider(void) {}

time_t supla_abstract_db_access_provider::mytime_to_time_t(MYSQL_TIME *time) {
  if (!time) {
    return 0;
  }

  struct tm timeinfo = {};
  timeinfo.tm_year = time->year - 1900;
  timeinfo.tm_mon = time->month - 1;
  timeinfo.tm_mday = time->day;
  timeinfo.tm_hour = time->hour;
  timeinfo.tm_min = time->minute;
  timeinfo.tm_sec = time->second;

  return mktime(&timeinfo);
}
