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

#ifndef TEMPERATURE_LOGGER_H_
#define TEMPERATURE_LOGGER_H_

#include <vector>

#include "datalogger/abstract_datalogger.h"

class supla_temperature_logger : public supla_abstract_datalogger {
 protected:
  virtual unsigned int log_interval_sec(void);
  virtual void log(const std::vector<supla_user *> *users,
                   supla_abstract_db_access_provider *dba);

 public:
  supla_temperature_logger();
  virtual ~supla_temperature_logger();
};

#endif /* TEMPERATURE_LOGGER_H_ */
