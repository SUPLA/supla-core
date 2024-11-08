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

#include "http/abstract_curl_adapter.h"

supla_abstract_curl_adapter::supla_abstract_curl_adapter(void) {}

supla_abstract_curl_adapter::~supla_abstract_curl_adapter(void) {}

void supla_abstract_curl_adapter::set_opt_url(const char *url) {
  set_opt_url(0, url);
}

void supla_abstract_curl_adapter::set_opt_post_fields(const char *fields) {
  set_opt_post_fields(0, fields);
}

void supla_abstract_curl_adapter::set_opt_write_data(std::string *data) {
  set_opt_write_data(0, data);
}

void supla_abstract_curl_adapter::set_opt_header_data(
    std::list<std::string> *data) {
  set_opt_header_data(0, data);
}

void supla_abstract_curl_adapter::set_opt_verbose(bool on) {
  set_opt_verbose(0, on);
}

void supla_abstract_curl_adapter::set_opt_custom_request(const char *method) {
  set_opt_custom_request(0, method);
}

bool supla_abstract_curl_adapter::append_header(const char *string) {
  return append_header(0, string);
}

bool supla_abstract_curl_adapter::perform(void) { return perform(0); }

long supla_abstract_curl_adapter::get_response_code(void) {
  return get_response_code(0);
}

std::string supla_abstract_curl_adapter::escape(const std::string &str) {
  return escape(0, str);
}
