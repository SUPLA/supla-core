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

#ifndef ABSTRACT_AUTO_GATE_CLOSING_H_
#define ABSTRACT_AUTO_GATE_CLOSING_H_

#include <vector>

#include "cyclictasks/abstract_auto_gate_closing_dao.h"
#include "cyclictasks/abstract_cyclictask.h"
#include "device/channel_property_getter.h"
#include "device/value/channel_gate_value.h"

class supla_abstract_auto_gate_closing : public supla_abstract_cyclictask {
 private:
  _gate_sensor_level_enum get_opening_sensor_level(
      supla_abstract_auto_gate_closing_dao::item_t *item);

 protected:
  virtual unsigned int task_interval_sec(void);
  virtual void run(const std::vector<supla_user *> *users,
                   supla_abstract_db_access_provider *dba);
  virtual bool user_access_needed(void);
  virtual supla_abstract_auto_gate_closing_dao *get_dao(
      supla_abstract_db_access_provider *dba) = 0;
  virtual void release_dao(supla_abstract_auto_gate_closing_dao *dao) = 0;
  virtual supla_abstract_channel_property_getter *get_property_getter(void) = 0;
  virtual void release_property_getter(
      supla_abstract_channel_property_getter *property_getter) = 0;
  virtual void close_the_gate(int user_id, int device_id, int channel_id) = 0;

 public:
  supla_abstract_auto_gate_closing(void);
  virtual ~supla_abstract_auto_gate_closing();
};

#endif /* ABSTRACT_AUTO_GATE_CLOSING_H_ */
