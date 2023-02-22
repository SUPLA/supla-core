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

#ifndef CURL_ADAPTER_H_
#define CURL_ADAPTER_H_

#include <curl/curl.h>

#include <string>

#include "http/abstract_curl_adapter.h"

class supla_curl_adapter : public supla_abstract_curl_adapter {
 private:
  CURL *curl;
  struct curl_slist *header;
  static size_t write_callback(void *contents, size_t size, size_t nmemb,
                               void *userp);

 public:
  supla_curl_adapter(void);
  virtual ~supla_curl_adapter(void);

  virtual void reset(void);
  virtual void set_opt_url(const char *url);
  virtual void set_opt_post_fields(const char *fields);
  virtual void set_opt_write_data(std::string *data);
  virtual void set_opt_verbose(bool on);
  virtual void set_opt_custom_request(const char *method);
  virtual bool append_header(const char *string);
  virtual bool perform(void);

  virtual long get_response_code(void);
};

#endif /* CURL_ADAPTER_H_ */
