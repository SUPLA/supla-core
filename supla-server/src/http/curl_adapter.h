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

#include <list>
#include <string>
#include <vector>

#include "http/abstract_curl_adapter.h"
#include "http/curl_instance.h"

class supla_curl_adapter : public supla_abstract_curl_adapter {
 private:
  supla_curl_instance *last_instance;
  std::vector<supla_curl_instance *> instances;
  supla_curl_instance *get_instance(int instance_id);

 public:
  supla_curl_adapter(void);
  virtual ~supla_curl_adapter(void);

  virtual void reset(void);
  virtual void set_opt_url(int instance_id, const char *url);
  virtual void set_opt_post_fields(int instance_id, const char *fields);
  virtual void set_opt_write_data(int instance_id, std::string *data);
  virtual void set_opt_header_data(int instance_id,
                                   std::list<std::string> *data);
  virtual void set_opt_verbose(int instance_id, bool on);
  virtual void set_opt_custom_request(int instance_id, const char *method);
  virtual bool append_header(int instance_id, const char *string);
  virtual bool perform(int instance_id);
  virtual long get_response_code(int instance_id);
  virtual std::string escape(int instance_id, const std::string &str);
  virtual void cancel(void);
};

#endif /* CURL_ADAPTER_H_ */
