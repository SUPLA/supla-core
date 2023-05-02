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

#ifndef VALUE_BASED_TRIGGER_CONDITION_RESULT_H_
#define VALUE_BASED_TRIGGER_CONDITION_RESULT_H_

#include <map>
#include <memory>
#include <string>

class supla_value_based_trigger;
class supla_vbt_condition_result {
 private:
  bool cnd_met;
  std::shared_ptr<supla_value_based_trigger> trigger;
  std::map<std::string, std::string> replacement_map;

 public:
  supla_vbt_condition_result(bool cnd_met);
  virtual ~supla_vbt_condition_result(void);

  const std::shared_ptr<supla_value_based_trigger> &get_trigger(void);
  void set_trigger(const std::shared_ptr<supla_value_based_trigger> &trigger);

  const std::map<std::string, std::string> &get_replacement_map(void);
  void set_replacement_map(
      const std::map<std::string, std::string> &replacement_map);

  bool are_conditions_met(void);
};

#endif /* VALUE_BASED_TRIGGER_CONDITION_RESULT_H_ */
