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

#include <assert.h>

#include <regex>

#include "log.h"
#include "svrcfg.h"

using std::list;
using std::regex;
using std::regex_replace;
using std::string;

supla_curl_adapter::supla_curl_adapter(void) : supla_abstract_curl_adapter() {
  write_data_ptr = nullptr;
  header_data_ptr = nullptr;
  header = nullptr;
  curl = curl_easy_init();
  assert(curl != nullptr);
}

supla_curl_adapter::~supla_curl_adapter(void) {
  if (header) {
    curl_slist_free_all(header);
    header = nullptr;
  }
  curl_easy_cleanup(curl);
}

// static
size_t supla_curl_adapter::write_callback(void *contents, size_t size,
                                          size_t nmemb, void *userp) {
  supla_curl_adapter *adapter = static_cast<supla_curl_adapter *>(userp);
  if (adapter && adapter->write_data_ptr && size * nmemb > 0) {
    adapter->write_data_ptr->append((char *)contents, size * nmemb);
  }

  return size * nmemb;
}

// static
size_t supla_curl_adapter::header_callback(void *contents, size_t size,
                                           size_t nmemb, void *userp) {
  supla_curl_adapter *adapter = static_cast<supla_curl_adapter *>(userp);

  if (adapter && adapter->header_data_ptr && size * nmemb > 0) {
    string item;
    item.append((char *)contents, size * nmemb);
    regex pattern(R"(\r\n$)");
    item = regex_replace(item, pattern, "");
    adapter->header_data_ptr->push_back(item);
  }

  return size * nmemb;
}

void supla_curl_adapter::reset(void) {
  curl_easy_reset(curl);
  write_data_ptr = nullptr;
  header_data_ptr = nullptr;

  if (header) {
    curl_slist_free_all(header);
    header = nullptr;
  }
}

void supla_curl_adapter::set_opt_url(const char *url) {
  this->url = url;
  curl_easy_setopt(curl, CURLOPT_URL, url);
}

void supla_curl_adapter::set_opt_post_fields(const char *fields) {
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, fields);
}

void supla_curl_adapter::set_opt_write_data(std::string *data) {
  write_data_ptr = data;
}

void supla_curl_adapter::set_opt_header_data(list<string> *data) {
  header_data_ptr = data;
}

void supla_curl_adapter::set_opt_verbose(bool on) {
  curl_easy_setopt(curl, CURLOPT_VERBOSE, on ? 1L : 0L);
}

void supla_curl_adapter::set_opt_custom_request(const char *method) {
  curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method);
}

string supla_curl_adapter::escape(const string &str) {
  char *escaped = curl_easy_escape(curl, str.c_str(), str.size());
  string result = escaped;
  curl_free(escaped);
  return result;
}

bool supla_curl_adapter::append_header(const char *string) {
  struct curl_slist *new_header = curl_slist_append(header, string);

  if (new_header == nullptr) {
    return false;
  } else {
    header = new_header;
    return true;
  }
}

bool supla_curl_adapter::perform(void) {
  if (header) {
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
  }

  curl_easy_setopt(curl, CURLOPT_USERAGENT, "supla-server");
  curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT,
                   scfg_int(CFG_HTTP_CONNECTION_TIMEOUT));
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,
                   supla_curl_adapter::write_callback);
  curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION,
                   supla_curl_adapter::header_callback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
  curl_easy_setopt(curl, CURLOPT_HEADERDATA, this);

  CURLcode result = curl_easy_perform(curl);

  if (CURLE_OK != result) {
    supla_log(LOG_ERR, "curl_easy_perform() failed: %s. %s",
              curl_easy_strerror(result), url.c_str());
    return false;
  }

  return true;
}

long supla_curl_adapter::get_response_code(void) {
  long http_code = 0;
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
  return http_code;
}
