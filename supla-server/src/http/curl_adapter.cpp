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

#include "log.h"

supla_curl_adapter::supla_curl_adapter(void) : supla_abstract_curl_adapter() {
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
  if (size * nmemb > 0 && userp) {
    ((std::string *)userp)->append((char *)contents, size * nmemb);
  }

  return size * nmemb;
}

void supla_curl_adapter::reset(void) {
  curl_easy_reset(curl);

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
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, data);
}

void supla_curl_adapter::set_opt_verbose(bool on) {
  curl_easy_setopt(curl, CURLOPT_VERBOSE, on ? 1L : 0L);
}

void supla_curl_adapter::set_opt_custom_request(const char *method) {
  curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method);
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
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,
                   supla_curl_adapter::write_callback);

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

__time_t supla_curl_adapter::get_timestamp(void) {
  struct timeval now;
  gettimeofday(&now, nullptr);
  return now.tv_sec;
}
