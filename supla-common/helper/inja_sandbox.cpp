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

#include "inja_sandbox.h"

#include <regex>
#include <stdexcept>

#include "device/extended_value/channel_em_extended_value.h"
#include "device/extended_value/channel_ic_extended_value.h"
#include "exception/abort_exception.h"

using std::move;
using std::regex;
using std::regex_search;
using std::runtime_error;
using std::string;

supla_inja_sandbox::supla_inja_sandbox(void) {
  getter = nullptr;
  max_blocks = 20;
  max_ifs = 15;
  allow_for = false;
  max_output_bytes = 1024;
}

supla_inja_sandbox::~supla_inja_sandbox(void) {}

// static
int supla_inja_sandbox::count_occ(const string &s, const string &needle) {
  int c = 0;
  size_t p = 0;
  while ((p = s.find(needle, p)) != string::npos) {
    ++c;
    p += needle.size();
  }
  return c;
}

string supla_inja_sandbox::validate_and_render(const string &tpl,
                                               const nlohmann::json &data) {
  env.set_search_included_templates_in_files(false);

  static const regex re_include(R"(\{\%\s*include\b)");
  if (regex_search(tpl, re_include)) {
    throw runtime_error("Include statements are not allowed.");
  }

  static const regex re_extends(R"(\{\%\s*extends\b)");
  if (regex_search(tpl, re_extends)) {
    throw runtime_error("Extends statements are not allowed.");
  }

  static const regex re_range_call(R"(\brange\s*\()");
  if (!allow_for && regex_search(tpl, re_range_call)) {
    throw runtime_error("The range() function is not allowed.");
  }

  if (count_occ(tpl, "{%") > max_blocks) {
    throw runtime_error("Too many control blocks in template.");
  }

  env.parse(tpl);

  string out = env.render(tpl, data);

  if (out.size() > max_output_bytes) {
    throw runtime_error("Template execution exceeded allowed output size (1024).");
  }

  return out;
}

void supla_inja_sandbox::register_abort_function(void) {
  env.add_callback("abort", 0, [](inja::Arguments &args) -> inja::json {
    throw abort_exception("Render aborted.");
  });
}

void supla_inja_sandbox::register_get_channel_function(
    supla_abstract_channel_property_getter *getter) {
  this->getter = getter;

  env.add_callback(
      "getChannel", 1, [this](inja::Arguments &args) -> inja::json {
        int channel_id = args.at(0)->get<int>();
        supla_channel_availability_status status;
        int func = 0;
        supla_abstract_channel_value *value = this->getter->get_value(
            this->getter->get_user_id(), 0, channel_id, &func, &status);

        nlohmann::json result;
        result = status.get_template_data();

        if (value) {
          if (supla_channel_ic_extended_value::is_function_supported(func) ||
              supla_channel_em_extended_value::is_function_supported(func)) {
            delete value;
            supla_abstract_channel_extended_value *extended_value =
                this->getter->get_extended_value(this->getter->get_user_id(), 0,
                                                 channel_id);

            if (extended_value) {
              result.update(extended_value->get_template_data());
              delete extended_value;
            }
          } else {
            result.update(value->get_template_data());
            delete value;
          }
        }

        return result;
      });
}
