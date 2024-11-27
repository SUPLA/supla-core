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

#include "http/curl_adapter.h"

using std::list;
using std::string;
using std::vector;

supla_curl_adapter::supla_curl_adapter(void) : supla_abstract_curl_adapter() {
  last_instance = nullptr;
}

supla_curl_adapter::~supla_curl_adapter(void) {
  for (auto it = instances.cbegin(); it != instances.cend(); ++it) {
    delete *it;
  }
}

supla_curl_instance *supla_curl_adapter::get_instance(int instance_id) {
  if (last_instance && last_instance->get_id() == instance_id) {
    return last_instance;
  }

  for (auto it = instances.cbegin(); it != instances.cend(); ++it) {
    if ((*it)->get_id() == instance_id) {
      last_instance = *it;
      return last_instance;
    }
  }

  last_instance = new supla_curl_instance(instance_id);
  instances.push_back(last_instance);
  return last_instance;
}

void supla_curl_adapter::reset(void) {
  for (auto it = instances.cbegin(); it != instances.cend(); ++it) {
    (*it)->reset();
  }
}

void supla_curl_adapter::set_opt_url(int instance_id, const char *url) {
  get_instance(instance_id)->set_opt_url(url);
}

void supla_curl_adapter::set_opt_post_fields(int instance_id,
                                             const char *fields) {
  get_instance(instance_id)->set_opt_post_fields(fields);
}

void supla_curl_adapter::set_opt_write_data(int instance_id,
                                            std::string *data) {
  get_instance(instance_id)->set_opt_write_data(data);
}

void supla_curl_adapter::set_opt_header_data(int instance_id,
                                             list<string> *data) {
  get_instance(instance_id)->set_opt_header_data(data);
}

void supla_curl_adapter::set_opt_verbose(int instance_id, bool on) {
  get_instance(instance_id)->set_opt_verbose(on);
}

void supla_curl_adapter::set_opt_custom_request(int instance_id,
                                                const char *method) {
  get_instance(instance_id)->set_opt_custom_request(method);
}

string supla_curl_adapter::escape(int instance_id, const string &str) {
  return get_instance(instance_id)->escape(str);
}

bool supla_curl_adapter::append_header(int instance_id, const char *string) {
  return get_instance(instance_id)->append_header(string);
}

bool supla_curl_adapter::perform(int instance_id) {
  return get_instance(instance_id)->perform();
}

long supla_curl_adapter::get_response_code(int instance_id) {
  return get_instance(instance_id)->get_response_code();
}

void supla_curl_adapter::cancel(void) {
  for (auto it = instances.cbegin(); it != instances.cend(); ++it) {
    (*it)->cancel();
  }
}
