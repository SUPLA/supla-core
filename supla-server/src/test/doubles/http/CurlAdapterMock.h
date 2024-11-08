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

#ifndef CURL_ADAPTER_MOCK_H_
#define CURL_ADAPTER_MOCK_H_

#include <gmock/gmock.h>

#include <list>
#include <string>

#include "http/abstract_curl_adapter.h"

namespace testing {

class CurlAdapterMock : public supla_abstract_curl_adapter {
 public:
  CurlAdapterMock(void);
  virtual ~CurlAdapterMock(void);

  MOCK_METHOD0(reset, void(void));
  MOCK_METHOD2(set_opt_url, void(int instance_id, const char *url));
  MOCK_METHOD2(set_opt_post_fields, void(int instance_id, const char *fields));
  MOCK_METHOD2(set_opt_header_data,
               void(int instance_id, std::list<std::string> *data));
  MOCK_METHOD2(set_opt_write_data, void(int instance_id, std::string *data));
  MOCK_METHOD2(set_opt_verbose, void(int instance_id, bool on));
  MOCK_METHOD2(set_opt_custom_request,
               void(int instance_id, const char *method));
  MOCK_METHOD2(append_header, bool(int instance_id, const char *string));
  MOCK_METHOD1(perform, bool(int instance_id));
  MOCK_METHOD1(get_response_code, long(int instance_id));
  MOCK_METHOD2(escape, std::string(int instance_id, const std::string &str));
  MOCK_METHOD0(cancel, void(void));
};

} /* namespace testing */

#endif /* CURL_ADAPTER_MOCK_H_ */
