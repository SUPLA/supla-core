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

#include "http/abstract_curl_adapter.h"

namespace testing {

class CurlAdapterMock : public supla_abstract_curl_adapter {
 private:
 protected:
 public:
  CurlAdapterMock(void);
  virtual ~CurlAdapterMock(void);

  MOCK_METHOD0(reset, void(void));
  MOCK_METHOD1(set_opt_url, void(const char *url));
  MOCK_METHOD1(set_opt_post_fields, void(const char *fields));
  MOCK_METHOD1(set_opt_write_data, void(std::string *data));
  MOCK_METHOD1(set_opt_verbose, void(bool on));
  MOCK_METHOD1(set_opt_custom_request, void(const char *method));
  MOCK_METHOD1(append_header, bool(const char *string));
  MOCK_METHOD0(perform, bool(void));
};

} /* namespace testing */

#endif /* CURL_ADAPTER_MOCK_H_ */
