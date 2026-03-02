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

#ifndef INJA_HELPER_H_
#define INJA_HELPER_H_

#include <inja/inja.hpp>
#include <nlohmann/json.hpp>
#include <string>

class supla_inja_helper {
 private:
  inja::Environment env;
  int max_blocks;
  int max_ifs;
  bool allow_for;
  size_t max_output_bytes;
  static int count_occ(const std::string& s, const std::string& needle);

 protected:
 public:
  supla_inja_helper(void);
  virtual ~supla_inja_helper(void);
  std::string validate_and_render(const std::string& tpl,
                                  const nlohmann::json& data);
  std::string validate_and_render(const std::string& tpl,
                                  std::map<std::string, std::string>* data);
};

#endif /* INJA_HELPER_H_*/
