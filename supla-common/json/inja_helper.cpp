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

#include "helper/inja_helper.h"

#include <regex>
#include <stdexcept>

using std::move;
using std::regex;
using std::regex_search;
using std::runtime_error;
using std::string;

supla_inja_helper::supla_inja_helper(void) {
  max_blocks = 20;
  max_ifs = 20;
  allow_for = false;
  max_output_bytes = 1024;
}

supla_inja_helper::~supla_inja_helper(void) {}

// static
int supla_inja_helper::count_occ(const string& s, const string& needle) {
  int c = 0;
  size_t p = 0;
  while ((p = s.find(needle, p)) != string::npos) {
    ++c;
    p += needle.size();
  }
  return c;
}

string supla_inja_helper::validate_and_render(const string& tpl,
                                              const nlohmann::json& data) {
  env.set_search_included_templates_in_files(false);

  static const regex re_include(R"(\{\%\s*include\b)");
  if (regex_search(tpl, re_include)) {
    throw runtime_error("Include statements are not allowed.");
  }

  static const regex re_extends(R"(\{\%\s*extends\b)");
  if (regex_search(tpl, re_extends)) {
    throw runtime_error("Extends statements are not allowed.");
  }

  static const regex re_for(R"(\{\%\s*for\b)");
  if (!allow_for && regex_search(tpl, re_for)) {
    throw runtime_error("For loops are not allowed.");
  }

  static const regex re_range_call(R"(\brange\s*\()");
  if (!allow_for && regex_search(tpl, re_range_call)) {
    throw runtime_error("The range() function is not allowed.");
  }

  if (count_occ(tpl, "{%") > max_blocks) {
    throw runtime_error("Too many control blocks in template.");
  }
  if (count_occ(tpl, "{% if") > max_ifs) {
    throw runtime_error("Too many if statements in template.");
  }

  env.parse(tpl);

  string out = env.render(tpl, data);

  if (out.size() > max_output_bytes) {
    throw runtime_error("Template execution exceeded allowed output size.");
  }

  return out;
}

string supla_inja_helper::validate_and_render(
    const std::string& tpl, std::map<std::string, std::string>* data) {
  nlohmann::json jdata;
  for (auto it = data->begin(); it != data->end(); ++it) {
    jdata[it->first] = it->second;
  }

  return validate_and_render(tpl, jdata);
}
