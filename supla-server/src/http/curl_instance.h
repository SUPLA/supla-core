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

#ifndef CURL_INSTANCE_H_
#define CURL_INSTANCE_H_

#include <curl/curl.h>

#include <list>
#include <string>

class supla_curl_instance {
 private:
  int id;
  std::string *write_data_ptr;
  std::list<std::string> *header_data_ptr;
  std::string url;
  CURL *curl;
  struct curl_slist *header;
  static size_t write_callback(void *contents, size_t size, size_t nmemb,
                               void *userp);
  static size_t header_callback(void *contents, size_t size, size_t nmemb,
                                void *userp);

 public:
  explicit supla_curl_instance(int id);
  virtual ~supla_curl_instance(void);

  int get_id(void);
  void reset(void);
  void set_opt_url(const char *url);
  void set_opt_post_fields(const char *fields);
  void set_opt_write_data(std::string *data);
  void set_opt_header_data(std::list<std::string> *data);
  void set_opt_verbose(bool on);
  void set_opt_custom_request(const char *method);
  bool append_header(const char *string);
  bool perform(void);
  long get_response_code(void);
  std::string escape(const std::string &str);
  void cancel(void);
};

#endif /* CURL_INSTANCE_H_ */
