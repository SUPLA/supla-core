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

#ifndef ABSTRACT_CURL_ADAPTER_H_
#define ABSTRACT_CURL_ADAPTER_H_

#include <list>
#include <string>

class supla_abstract_curl_adapter {
 public:
  supla_abstract_curl_adapter(void);
  virtual ~supla_abstract_curl_adapter(void);

  virtual void reset(void) = 0;
  virtual void set_opt_url(const char *url) = 0;
  virtual void set_opt_post_fields(const char *fields) = 0;
  virtual void set_opt_write_data(std::string *data) = 0;
  virtual void set_opt_header_data(std::list<std::string> *data) = 0;
  virtual void set_opt_verbose(bool on) = 0;
  virtual void set_opt_custom_request(const char *method) = 0;
  virtual bool append_header(const char *string) = 0;
  virtual bool perform(void) = 0;
  virtual long get_response_code(void) = 0;
  virtual std::string escape(const std::string &str) = 0;
};

#endif /* ABSTRACT_CURL_ADAPTER_H_ */
